// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — per-symbol aggregates for the selector.

/// @file aggregates.hpp
/// @brief Per-symbol rolling aggregates shared between the screener
///        (producer) and the selector (consumer).
///
/// Step 5 baseline: only an `events_total` proxy from `!bookTicker`.
///
/// F5 extends this with bucketed rolling time-frame windows for volume
/// and trade-count, fed from the 1 Hz `!ticker@arr` stream. Each
/// window holds N fixed-size buckets; on `update(now, dv, dt)` it
/// slides the head forward (zero-filling skipped buckets) and adds
/// the delta into the current bucket. `vol_sum()` / `trades_sum()`
/// return the rolling totals.

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace binapi2::examples::async_recorder {

/// @brief Bucketed rolling window of `(volume, trades)` pairs.
class tf_window
{
public:
    using clock = std::chrono::steady_clock;

    tf_window(std::size_t n_buckets, std::chrono::seconds bucket_size) :
        bucket_seconds_(bucket_size),
        buckets_(n_buckets, bucket{}),
        head_(0),
        head_time_{}
    {
    }

    /// @brief Slide the window forward to `now`, zero-filling any
    ///        skipped buckets, then add the delta into the head bucket.
    void update(clock::time_point now, double dv, double dt)
    {
        advance_to(now);
        buckets_[head_].vol += dv;
        buckets_[head_].trades += dt;
    }

    /// @brief Total volume across all buckets currently in the window.
    [[nodiscard]] double vol_sum() const
    {
        double s = 0.0;
        for (const auto& b : buckets_) s += b.vol;
        return s;
    }

    /// @brief Total trade count across all buckets.
    [[nodiscard]] double trades_sum() const
    {
        double s = 0.0;
        for (const auto& b : buckets_) s += b.trades;
        return s;
    }

    /// @brief Number of buckets in the window. Window length is
    ///        `n_buckets() * bucket_seconds()`.
    [[nodiscard]] std::size_t n_buckets() const noexcept { return buckets_.size(); }
    [[nodiscard]] std::chrono::seconds bucket_seconds() const noexcept { return bucket_seconds_; }

private:
    struct bucket
    {
        double vol{ 0.0 };
        double trades{ 0.0 };
    };

    void advance_to(clock::time_point now)
    {
        if (head_time_.time_since_epoch().count() == 0) {
            head_time_ = now;
            return;
        }
        if (now <= head_time_) return;

        const auto delta_secs = std::chrono::duration_cast<std::chrono::seconds>(
                                    now - head_time_).count();
        const auto bucket_s = bucket_seconds_.count();
        const auto steps = delta_secs / bucket_s;
        if (steps <= 0) return;

        // Cap at n_buckets — a longer gap means the entire window
        // contained only stale data and should be wiped.
        const auto n = static_cast<long long>(buckets_.size());
        const auto effective = std::min<long long>(steps, n);
        for (long long i = 0; i < effective; ++i) {
            head_ = (head_ + 1) % buckets_.size();
            buckets_[head_] = {};
        }
        head_time_ += std::chrono::seconds(steps * bucket_s);
    }

    std::chrono::seconds bucket_seconds_;
    std::vector<bucket> buckets_;
    std::size_t head_;
    clock::time_point head_time_;
};

/// @brief In-progress synthetic 1-minute kline. Populated from
///        per-symbol `last_price` samples (ticker@arr) and optionally
///        extended on intra-sample high/low by bookTicker mid prices.
///
/// `minute_index` is the integer minute since Unix epoch
/// (`event_time_ms / 60000`). A new sample whose minute index differs
/// from the current bar's means the bar should be closed.
struct bar_1m
{
    std::uint64_t minute_index{ 0 };
    double open{ 0.0 };
    double high{ 0.0 };
    double low{ 0.0 };
    double close{ 0.0 };
    bool primed{ false };

    /// @brief Reset to a freshly-opened bar at `price`.
    void reopen(std::uint64_t minute, double price)
    {
        minute_index = minute;
        open = price;
        high = price;
        low = price;
        close = price;
        primed = true;
    }

    /// @brief Extend high/low on an intra-bar observation (any source).
    void extend_range(double price)
    {
        if (price > high) high = price;
        if (price < low)  low  = price;
    }

    /// @brief Update open/close semantics from the authoritative trade
    ///        price source (ticker's `last_price`). The first sample
    ///        after reopen/prime carries open; every sample carries
    ///        close; high/low are extended.
    void update_from_trade(double price)
    {
        close = price;
        extend_range(price);
    }
};

/// @brief Ring buffer of the last N true-range values. Mean across the
///        primed window is the ATR(N) estimate.
class tr_window
{
public:
    explicit tr_window(std::size_t n) :
        slots_(n, 0.0), head_(0), count_(0)
    {
    }

    void push(double tr)
    {
        slots_[head_] = tr;
        head_ = (head_ + 1) % slots_.size();
        if (count_ < slots_.size()) ++count_;
    }

    [[nodiscard]] bool primed() const noexcept
    {
        return count_ == slots_.size();
    }

    [[nodiscard]] double atr() const
    {
        if (count_ == 0) return 0.0;
        double s = 0.0;
        for (std::size_t i = 0; i < count_; ++i) s += slots_[i];
        return s / static_cast<double>(count_);
    }

    [[nodiscard]] std::size_t size() const noexcept { return slots_.size(); }
    [[nodiscard]] std::size_t count() const noexcept { return count_; }

private:
    std::vector<double> slots_;
    std::size_t head_;
    std::size_t count_;
};

/// @brief Per-symbol rolling stats. Read by the selector, written by
///        the screener. Same-executor-only — no synchronisation.
struct symbol_agg
{
    /// @brief Monotonic count of bookTicker events observed for this
    /// symbol since startup. Cheap "is this symbol live" proxy and
    /// fallback when the TF windows are still warming up.
    std::size_t events_total{ 0 };

    /// @brief Cached 24h rolling totals from the most recent
    /// `!ticker@arr` entry for this symbol. Used to compute the
    /// per-second deltas that feed the windows.
    double last_quote_volume{ 0.0 };
    std::uint64_t last_trade_count{ 0 };
    bool primed{ false };

    /// @brief Rolling volume / trade count windows.
    tf_window win_1m{ 60, std::chrono::seconds(1) };   // 60 × 1 s = 1 min
    tf_window win_5m{ 60, std::chrono::seconds(5) };   // 60 × 5 s = 5 min
    tf_window win_1h{ 60, std::chrono::seconds(60) };  // 60 × 60 s = 1 h

    [[nodiscard]] double vol_1m() const { return win_1m.vol_sum(); }
    [[nodiscard]] double vol_5m() const { return win_5m.vol_sum(); }
    [[nodiscard]] double vol_1h() const { return win_1h.vol_sum(); }
    [[nodiscard]] double trades_1m() const { return win_1m.trades_sum(); }
    [[nodiscard]] double trades_5m() const { return win_5m.trades_sum(); }
    [[nodiscard]] double trades_1h() const { return win_1h.trades_sum(); }

    // -- Synthetic 1-minute klines + NATR (F6) ------------------------------
    /// @brief In-progress 1m bar. Open/close come from ticker@arr
    ///        last_price; high/low may be extended by bookTicker mid.
    bar_1m current_bar{};
    /// @brief Close of the previously-closed bar. Used by the TR term
    ///        `|high - prev_close|` / `|low - prev_close|`.
    double prev_close{ 0.0 };
    /// @brief Rolling True Range window. 14 slots gives an ATR(14).
    tr_window trw{ 14 };
    /// @brief Latest NATR percentage (`atr / close * 100`). Zero until
    ///        the first bar has closed. Not primed until `trw.primed()`.
    double natr{ 0.0 };

    /// @brief Compute and record the bar's True Range, then roll the
    ///        ATR/NATR and reset `current_bar` to a new one starting
    ///        at `next_minute_index` and `next_price`.
    void close_bar_and_open_next(std::uint64_t next_minute_index,
                                 double next_price)
    {
        if (current_bar.primed) {
            const double range = current_bar.high - current_bar.low;
            const double hi_gap = (prev_close > 0.0)
                ? std::fabs(current_bar.high - prev_close) : 0.0;
            const double lo_gap = (prev_close > 0.0)
                ? std::fabs(current_bar.low - prev_close) : 0.0;
            const double tr = std::max({ range, hi_gap, lo_gap });
            trw.push(tr);
            prev_close = current_bar.close;

            const double atr = trw.atr();
            natr = (current_bar.close > 0.0 && atr > 0.0)
                ? (atr / current_bar.close) * 100.0
                : 0.0;
        }
        current_bar.reopen(next_minute_index, next_price);
    }
};

/// @brief Map from uppercase symbol string to its aggregate slot.
using aggregates_map = std::unordered_map<std::string, symbol_agg>;

} // namespace binapi2::examples::async_recorder
