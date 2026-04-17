// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file config.hpp
/// @brief Connection and authentication settings for the Binance USD-M Futures
///        API (REST, WebSocket-API, and market-data streams).

#pragma once

#include <binapi2/fapi/transport_logger.hpp>

#include <cstdint>
#include <string>

namespace binapi2::fapi {

/// @brief Signing algorithm for authenticated requests.
enum class sign_method_t
{
    ed25519, ///< Ed25519 asymmetric signing (recommended, required for WS API session.logon).
    hmac,    ///< HMAC-SHA256 symmetric signing (deprecated by Binance).
};


/// WebSocket target path (e.g. "/ws/btcusdt@bookTicker").
/// Constructed by stream_traits::target() from a subscription type.
using ws_target_t = std::string;

/// @brief Complete configuration for a Binance USD-M Futures client session.
///
/// Default-constructed values point to the **production** endpoints.  Use
/// `testnet_config()` to obtain a config pre-filled with testnet hosts.
/// Callers must set `api_key` and `secret_key` before any authenticated call.
struct config
{
    // -- REST API endpoints --------------------------------------------------
    std::string rest_host{ "fapi.binance.com" };
    std::string rest_port{ "443" };
    std::string rest_base_path{};

    // -- WebSocket API endpoints (request/response over WS) ------------------
    std::string websocket_api_host{ "ws-fapi.binance.com" };
    std::string websocket_api_port{ "443" };
    std::string websocket_api_target{ "/ws-fapi/v1" };

    // -- Market-data stream endpoints ----------------------------------------
    std::string stream_host{ "fstream.binance.com" };
    std::string stream_port{ "443" };
    std::string stream_base_target{ "/ws" };
    std::string combined_stream_target{ "/stream" };

    // -- Authentication & client settings ------------------------------------
    std::string api_key{};
    std::string secret_key{};

    /// @brief PEM-encoded Ed25519 private key for request signing.
    /// Required when `sign_method == sign_method_t::ed25519`.
    std::string ed25519_private_key_pem{};

    /// @brief Signing algorithm. Defaults to Ed25519 (recommended by Binance).
    sign_method_t sign_method{ sign_method_t::ed25519 };

    /// @brief Server-side tolerance window (ms) for timestamp validation.
    /// Binance rejects requests whose timestamp differs from server time by
    /// more than this value.
    std::uint64_t recv_window{ 5000 };

    std::string user_agent{ "binapi2-fapi/0.1.0" };

    /// @brief Optional path to a CA certificate file (PEM) for TLS verification.
    /// When set, the transport uses this file instead of (in addition to) the
    /// system default CA paths. Useful for self-signed certificates (e.g. mock
    /// servers) or custom CA bundles.
    std::string ca_cert_file{};

    /// @brief Connect timeout (seconds) for WebSocket and HTTP connections.
    /// Each step of the connect sequence (DNS resolve, TCP connect, TLS
    /// handshake, WS upgrade) must complete within this budget.  If any
    /// step takes longer, the connect attempt fails with a timeout error
    /// and the subscribe retry loop can try again.  0 = no timeout.
    int connect_timeout_seconds{ 10 };

    /// @brief Optional callback for transport-level logging.
    ///
    /// When set, the HTTP and WebSocket transport layers invoke this callback
    /// for every message sent or received, providing the raw method, target,
    /// status, and body.  When null (default), no logging overhead is incurred.
    transport_logger logger{};

    /// @brief Optional callback invoked for every raw WebSocket frame received
    ///        during a stream read loop, before JSON parsing.
    ///
    /// Useful for recording raw stream data to a file for debugging or replay.
    /// The callback receives the raw text payload exactly as received from the
    /// server. When null (default), no overhead is incurred.
    /// @brief When true the config targets the Binance Futures testnet.
    bool testnet{ false };

    /// @brief Factory that returns a config pre-filled for the Binance Futures
    ///        **testnet** environment.
    ///
    /// The returned config still requires `api_key` and `secret_key` to be set
    /// by the caller.
    /// @return A config with testnet hosts and `testnet == true`.
    [[nodiscard]] static config testnet_config()
    {
        config cfg;
        cfg.rest_host = "demo-fapi.binance.com";
        cfg.websocket_api_host = "testnet.binancefuture.com";
        cfg.websocket_api_target = "/ws-fapi/v1";
        cfg.stream_host = "fstream.binancefuture.com";
        cfg.testnet = true;
        return cfg;
    }
};

} // namespace binapi2::fapi
