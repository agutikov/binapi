# `async-demo-ui` — FTXUI demonstration client

A second example binary alongside `async-demo-cli`. Same command surface
(REST, WS API, market streams, local order book) but rendered as an
interactive terminal UI built on **FTXUI 6.x**.

This document is the implementation plan. It is updated as steps complete.

---

## 1. Goal

Wrap every command exposed by `async-demo-cli` in a TUI surface that lets the
user:

* Pick a command from a grouped list, fill in its parameters via input fields,
  run it, and see the **request, response, and parsed JSON** in three views
  (raw HTTP, formatted JSON, FTXUI tree).
* Subscribe to a market stream with start/stop control and view events as
  either a **raw JSON list**, a **typed table**, or a **counter-only
  status line**.
* Run the local order book and see live ASKs/BIDs (later: a trades tape).

Both binaries link the same `binapi2_fapi` library and `secret_provider`. The
TUI does **not** reuse `common.hpp` or `cmd_*.cpp` from the CLI, because those
are CLI11-shaped and print through spdlog. The reuse boundary is just the
public API (`futures_usdm_api`, request/response types, executors).

---

## 2. Where step 0 left us

Step 0 is **complete** and merged into `examples/binapi2/fapi/async-demo-ui/`.
It establishes the full architecture so subsequent steps only fill in command
bodies.

```
examples/binapi2/fapi/async-demo-ui/
├── CMakeLists.txt          # links binapi2_fapi, secret_provider, ftxui::*
├── app_state.hpp           # shared state struct (atomics + mutex strings)
├── worker.hpp              # io_context thread + cobalt::spawn helper
├── worker.cpp
├── main.cpp                # CLI11 launch flag, FTXUI Screen.Loop, tab layout
└── views/
    ├── views.hpp           # one make_*() factory per tab
    ├── status_bar.cpp      # env / creds / jobs / status line
    ├── rest_view.cpp       # placeholder
    ├── ws_view.cpp         # placeholder
    ├── streams_view.cpp    # placeholder
    └── orderbook_view.cpp  # placeholder
```

What works at step 0:

* `binapi2-fapi-async-demo-ui [-l|--live]` launches the TUI in fullscreen.
* Status bar shows the active environment (`TESTNET` / `LIVE`), credential
  state (`loading… → ok / failed`), and active-job counter.
* Top-level tabs (`REST`, `WS API`, `Streams`, `Order Book`) cycle via Tab
  / arrow keys; each tab body renders a placeholder.
* `q` (or Ctrl-C) quits cleanly: the FTXUI loop exits, `worker::stop()` drops
  the work guard, the io_context drains, and the worker thread joins.
* Credentials load asynchronously on the io_context thread via
  `boost::cobalt::spawn(executor, task, use_future)` (the future is detached
  immediately — the task notifies the UI via `screen.PostEvent(Event::Custom)`
  on completion).

---

## 3. Architecture (refined after step 0)

### 3.1 Threading model

```
┌────────────── main thread ──────────────┐    ┌────────── worker thread ──────────┐
│                                         │    │                                   │
│  ftxui::ScreenInteractive::Loop()       │    │  boost::asio::io_context::run()   │
│    ├ input handling                     │    │    drives:                        │
│    ├ render (reads app_state snapshots) │    │      cobalt coroutines spawned    │
│    └ wakes on screen.PostEvent(...)     │    │      via cobalt::spawn(exec, t,   │
│                                         │◀──▶│                use_future)        │
│  app_state (atomics + small mutex strs) │    │                                   │
│  request_capture / stream_capture / …   │    │  writes into capture structs      │
│                                         │    │  notify_ui()  ─► PostEvent        │
└─────────────────────────────────────────┘    └───────────────────────────────────┘
```

Why this shape:

* **FTXUI owns the main thread.** `App::Loop(Component)` is a blocking
  call. We can't have `cobalt::main` here.
* **Cobalt is single-executor by design.** A *single* worker thread running
  one io_context drives all binapi coroutines, which matches the rest of the
  codebase (the CLI, the recorder).
* **`screen.PostEvent(Event::Custom)` is thread-safe** in FTXUI 6 — it pushes
  a custom event into the screen's input queue. We use it as the UI's
  redraw doorbell: render reads the latest snapshot under a short mutex.

### 3.2 Cancellation

Two coroutines need stop control:

* **Streams** — `exec_stream<Sub>` loops `co_await gen` forever. We need a
  Stop button that breaks the loop without killing the io_context.
* **Local order book** — already exposes `local_order_book::stop()` which
  signals the run loop. We just call it from the UI thread (it's mutex-
  protected internally).

For streams, we wrap the loop in a custom helper that races the next event
against an `asio::steady_timer` armed by a `cancellation_signal`, or — simpler
— use cobalt's `boost::asio::cancellation_signal` slot on the spawned task.
Resolve the exact mechanism in **step 3** when the first stream goes in; the
plan covers either choice.

### 3.3 State model — capture structs

Each "in-flight job" gets one capture. Captures live in `app_state` (or in
view-local state, depending on lifetime). All have the same shape: a mutex
and counters/atomics so the render path never blocks on slow IO.

```cpp
// REST or WS-API one-shot
struct request_capture
{
    enum state_t { idle, running, done, failed };
    std::atomic<state_t> state{ idle };

    mutable std::mutex   mtx;
    std::string          raw_request;     // transport_log_entry::raw  (sent)
    std::string          raw_response;    // transport_log_entry::raw  (received)
    std::string          pretty_json;     // glz::write<{prettify=true}>
    std::shared_ptr<glz::json_t> parsed;  // for the tree view
    std::string          error_message;
    int                  http_status = 0;
    int                  binance_code = 0;
};

// Stream subscription
struct stream_capture
{
    std::atomic<bool>           running{ false };
    std::atomic<std::uint64_t>  total_events{ 0 };
    std::atomic<std::uint64_t>  errors{ 0 };
    std::chrono::steady_clock::time_point last_event;

    mutable std::mutex          mtx;
    std::deque<std::string>     pretty_ring;  // bounded ~500 entries
    boost::asio::cancellation_signal cancel;
};

// Local order book
struct book_capture
{
    std::atomic<bool>          running{ false };
    std::atomic<std::uint64_t> updates{ 0 };

    mutable std::mutex                mtx;
    binapi2::fapi::order_book::order_book_snapshot latest;
    std::string                       symbol;
    int                               depth = 1000;
    binapi2::fapi::order_book::local_order_book* book = nullptr; // owned by task
};
```

### 3.4 Capturing requests/responses

`binapi2::fapi::config::logger` is a callback of type
`std::function<void(const transport_log_entry&)>`. The CLI sets this once at
startup and writes to spdlog. The UI does the same, but the callback writes
into the *currently active* `request_capture` — keyed by some "current call
id" so concurrent calls don't cross.

**Mechanism**: `worker` owns a `std::atomic<request_capture*> current_capture`.
Before spawning a request task, the view sets it; the logger callback writes
into it; the task clears it on completion. Because the io_context is single-
threaded, "currently active" is well-defined for the duration of one
sequential REST call. (For interleaved WS-API calls we'd need an id map —
out of scope until that becomes necessary.)

### 3.5 Capturing parsed JSON

`glaze` already produces pretty JSON via
`glz::write<glz::opts{ .prettify = true }>(value)`.

For the **tree view** we re-parse the response body once into a
`glz::json_t` (variant of object/array/string/number/bool/null). Walking it
is straightforward recursion. We render the tree using **`Collapsible`**
components — FTXUI ships `Collapsible(label, child)` exactly for this
purpose. Object/array nodes become collapsibles; leaves become plain text
labels.

**Depth limit**: cap rendering at 4 levels. Beyond that, show
`"… {N more keys}"` so `exchange-info` doesn't explode the component
tree. (The raw and pretty views are unlimited.)

### 3.6 Code-reuse boundary

The CLI and the UI both consume a new shared library
`binapi2_demo_commands` (extracted in **step 0.5**, see §4). The library
holds everything that's the same regardless of which surface drives it.

**Shared library `libs/binapi2_demo_commands/`**:

* `result_sink` — abstract interface (`on_request_start`, `on_transport`,
  `on_response_json`, `on_error`, `on_done`). The CLI ships a
  `spdlog_sink` implementation; the UI ships a `capture_sink` writing into
  `request_capture`.
* Sink-parameterised executors `exec_market_data` / `exec_account` /
  `exec_trade` / `exec_ws_public` / `exec_ws_signed` / `exec_stream` —
  same names as today's `common.hpp`, but free of globals (`verbosity`,
  spdlog) and parameterised on a `result_sink&`.
* **POD option structs** (`symbol_opts`, `kline_opts`, `order_opts`,
  `download_id_opts`, …) — currently duplicated as anonymous structs
  inside each surface's command files. Lifted into one header.
* **Request builders** — small free functions `make_klines_request(const
  kline_opts&) -> klines_request_t` etc. Both surfaces fill the same opts
  struct via their own form/option binding, then call the same builder.
* `parse_enum<E>(string_view)` — lifted from `async-demo-cli/common.hpp`.

**Per-binary, not shared**:

* The CLI11 subcommand registration tables (`add_subcommand`,
  `add_option`, `->required()` calls) — inherently surface-specific.
* The FTXUI form factories (`Input`, `Toggle`, `Container::Vertical`) —
  inherently surface-specific.

The split is: option *data* + request-build *logic* + transport
*executors* are shared; the *binding* of those to a CLI parser or a TUI
form is per-surface. Trying to abstract the binding (declare "this
command has a required `symbol` parameter" once and have both surfaces
consume it) would either need glaze-meta reflection (heavy + inflexible)
or its own DSL — more code than it saves.

---

## 4. Per-step plan

Each step has a clear acceptance criterion: the binary builds clean, the
described feature works end-to-end, and the previous steps still work.

### Step 0 — skeleton (DONE)

Already described above.

---

### Step 0.5 — extract `binapi2_demo_commands` library, migrate the CLI

**Goal**: stop the CLI from being the only consumer of the soon-to-be-
shared logic. Pull the reusable pieces out *before* the UI starts
duplicating them. After this step the CLI behaves identically but its
implementation is split between the new library and a thin CLI11 binding
layer.

**New library**:

```
libs/binapi2_demo_commands/
├── CMakeLists.txt
└── include/binapi2/demo_commands/
    ├── result_sink.hpp        # abstract sink interface (5 virtual fns)
    ├── spdlog_sink.hpp        # CLI implementation (writes to spdlog)
    ├── exec.hpp               # exec_market_data / exec_account / exec_trade
    │                          #   / exec_ws_public / exec_ws_signed / exec_stream
    │                          #   — sink-parameterised, no globals
    ├── enum_parse.hpp         # parse_enum<E>(string_view)
    ├── opts.hpp               # the POD option structs:
    │                          #   symbol_opts, symbol_limit_opts,
    │                          #   kline_opts, analytics_opts,
    │                          #   order_opts, symbol_order_id_opts,
    │                          #   download_id_opts, download_link_opts, …
    └── builders.hpp           # make_*_request(opts) -> Request
                               #   one per parameter shape (~10 builders)
```

**CLI changes** (in `examples/binapi2/fapi/async-demo-cli/`):

* `common.hpp` shrinks: `parse_enum`, `exec_*` templates, helper
  `add_kline_sub` / `add_pair_kline_sub` / `add_analytics_sub` /
  `add_download_id_sub` / `add_download_link_sub` are deleted (they move
  into `binapi2_demo_commands`). What stays: the spdlog plumbing,
  `make_config`, `async_load_secrets`, `verbosity`/`use_testnet` globals,
  print helpers (now wrapping the spdlog sink).
* Each `cmd_*.cpp` keeps its CLI11 binding code but calls the shared
  builders + executors. The local anonymous `add_noarg` / `add_symbol` /
  `add_symbol_limit` etc. helper templates stay (each binds the shared
  opts struct + shared builder to a CLI11 subcommand). The bodies of
  those helpers shrink to ~5 lines apiece because they no longer build
  the request inline.
* `selected_cmd::factory` becomes a `cmd_fn` that takes an
  `(api&, result_sink&)` so main.cpp constructs a `spdlog_sink` once and
  passes it down.

**CMake**:

* New top-level `add_subdirectory(libs/binapi2_demo_commands)` (alongside
  the existing `libs/secret_provider`).
* `binapi2-fapi-async-demo-cli` adds `binapi2_demo_commands` to its
  link line.

**Acceptance**:

* `./build.sh` clean.
* `./run_tests.sh` clean.
* Manual smoke: `scripts/testnet/market_data.sh` and a few representative
  commands behave identically (same JSON, same exit codes, same stdout
  formatting under `-v`/`-vv`/`-vvv`). No behaviour change is the whole
  point — this step is pure refactor.

**Non-goals**:

* Touching the UI (still at step 0). The library exists, is consumed by
  the CLI, and is sitting there ready for step 1.
* Designing form-binding abstractions for the UI side. The UI will write
  its own per-surface binding code in step 1+ — the library only shares
  data + logic, not surface code.

---

### Step 1 — first REST request

**Goal**: wire `ping` end-to-end so that one click in the UI runs the
coroutine, captures the response, and shows it in the response pane.

This is the first consumer of `binapi2_demo_commands` from the UI side.
The library's `exec_market_data` already exists (created in step 0.5);
step 1 only adds the **UI-side capture sink** and the FTXUI form/response
plumbing.

**New files** (all under `examples/binapi2/fapi/async-demo-ui/`):

```
util/
  capture_sink.hpp     # implements result_sink, fills a request_capture
  request_capture.hpp  # the struct described in §3.3
  json_tree.hpp        # walks glz::json_t into FTXUI Collapsibles
rest/
  ping_command.cpp     # the first command — defines its widget + run fn
views/rest_view.cpp    # replaced: command list (just "ping") + Run button
                       # + 3-tab response pane (Raw / JSON / Tree)
```

**capture_sink** implements `binapi2::demo::result_sink` (from the
shared library). Each method takes the lock on its bound
`request_capture`, writes the field, and calls `worker.notify_ui()`.

The view's Run button spawns the existing shared executor:

```cpp
auto sink = std::make_shared<demo_ui::capture_sink>(cap, wrk);
boost::cobalt::spawn(
    wrk.io().get_executor(),
    binapi2::demo::exec_market_data(api, types::ping_request_t{}, *sink),
    boost::asio::use_future);
```

(The `shared_ptr<sink>` is captured by the spawned task — it stays alive
until the coroutine completes.)

**Response pane internals**:

* `Container::Tab({raw_view, json_view, tree_view}, &response_tab_index)`
* `raw_view`: `Renderer` reading `cap.raw_request + "\n\n" + cap.raw_response`
  inside `vbox | yframe | vscroll_indicator`.
* `json_view`: same shape, reading `cap.pretty_json`.
* `tree_view`: a `Renderer` (or composition of `Collapsible`s) walking
  `cap.parsed`.

**Acceptance**:

* `ping` button shows "pong" status. JSON tab shows `{}` (ping has no body).
  Raw tab shows the actual `GET /fapi/v1/ping HTTP/1.1` and the 200 OK
  response with headers. Tree tab shows an empty object.
* While the request is in flight, status bar `jobs:` counter goes 0→1→0.
* Errors (e.g. forced bad URL) populate `cap.error_message` and the response
  pane shows the error in red.

---

### Step 2 — simple local order book

**Goal**: a working but minimal order book view, separate from the rest of
the REST work. Picks an early target so we exercise the order book code
path before all the REST commands are wired.

**New files**:

```
book/
  book_capture.hpp     # struct described in §3.3
views/orderbook_view.cpp  # symbol input, depth input, Start/Stop buttons,
                          # ASKs + BIDs panel (10 levels each side)
```

`local_order_book` already lives in `binapi2_fapi`; nothing needs to be
shared via `binapi2_demo_commands` for it (the book isn't a request, it
takes its own callback). The view spawns the book directly via
`boost::cobalt::spawn`.

**Behaviour**:

* User types `BTCUSDT`, depth `1000`, hits Start.
* The view spawns a cobalt task that constructs a `local_order_book`,
  installs `set_snapshot_callback` to mutate `book_capture::latest` under
  mutex and call `worker.notify_ui()`, then calls `co_await book.async_run(…)`.
* Stop button calls `book->stop()`.
* Render path takes the mutex, copies the top 10 ASKs and BIDs, displays
  them as a centred two-column block with the spread highlighted.

**Acceptance**:

* Start spins up the book; first snapshot appears within ~1 s on testnet.
* Updates ~10/s, screen tears nothing (PostEvent throttled to ≤30 Hz —
  see §6 risks).
* Stop returns to idle in <1 s, no crash, can Start again.

---

### Step 3 — first market data stream

**Goal**: prove the stream pattern (start, tick events into a ring buffer,
render counter line + last few JSON entries, stop).

**New files**:

```
streams/
  stream_capture.hpp        # struct described in §3.3
  stream_capture_sink.hpp   # implements demo::result_sink for streams
                            #   on_response_json -> push to ring buffer
views/streams_view.cpp      # subscription form (just bookTicker for now),
                            # Start/Stop button, counter line, raw JSON list
```

`exec_stream<Sub>` already lives in `binapi2_demo_commands` (created in
step 0.5, sink-parameterised). Step 3 adds the **stream-flavoured sink**:
each `on_response_json(pretty)` call in the sink pushes onto the
capture's bounded ring buffer instead of writing to spdlog. The view's
Start button does:

```cpp
auto sink = std::make_shared<demo_ui::stream_capture_sink>(cap, wrk);
boost::cobalt::spawn(
    wrk.io().get_executor(),
    binapi2::demo::exec_stream(api, sub, *sink),
    boost::asio::use_future);
```

The stream-flavoured sink implements PostEvent throttling (track last
notify time; only call `wrk.notify_ui()` if ≥ 33 ms elapsed). This
avoids drowning FTXUI under high-frequency streams.

**Cancellation**: the simplest cobalt pattern is an `std::atomic<bool>
stop` on `stream_capture`, checked by `exec_stream` in the shared
library between events. The Stop button sets the flag. To support this
cleanly, `binapi2::demo::exec_stream` takes an optional `stop_token`
parameter (an `std::atomic<bool>*`); the CLI passes `nullptr`. We don't
depend on cobalt's cancellation slot machinery — revisit if it becomes
limiting.

**Acceptance**:

* Subscribing to `bookTicker` for `BTCUSDT` shows events flowing into the
  list, counter ticks at the actual feed rate (~5–10/s).
* Stop drains immediately (next event is the last).
* Restart works.
* Switching tabs doesn't lose events while running (the task is independent
  of the view; the view just renders a snapshot).

---

### Step 4 — all REST requests

**Goal**: every command currently in `cmd_market_data.cpp`, `cmd_account.cpp`,
`cmd_trade.cpp`, `cmd_convert.cpp` becomes a UI command.

**Approach taken** (diverged from the original plan): instead of per-command
form factories returning `Component`, the view keeps a single shared
`form_state` (strings for symbol / pair / limit / interval / period) and a
function-pointer registry:

```cpp
struct rest_command {
    const char* name;
    const char* description;
    form_kind form;            // enum: no_args, symbol, symbol_opt,
                               //       symbol_limit, pair, kline,
                               //       pair_kline, analytics, basis
    void (*run)(const cmd_ctx&);
};
```

Each input widget is a single `Input` wrapped in `Maybe(...)`, keyed on whether
the selected command's `form_kind` uses that field. The focus chain is built
once; hidden inputs drop out of tab navigation automatically.

Per-command `run_fn` functions are tiny — they read the fields their form_kind
advertises, build a typed request, and delegate to a generic `run_cmd<Req>`
which resets the capture, builds a sink, prefills, and `cobalt::spawn`s a
templated `run_market_data<Req>` free-function coroutine.

Reusing the CLI's shape helpers (`make_symbol_request`, `make_kline_request`,
etc.) was considered, but the UI's build sites are already 1–3 lines each —
the builders add indirection without shrinking the code. Cross-surface code
reuse stays at the `exec_market_data / exec_account / exec_trade` /
`result_sink` boundary.

**Status — all four groups wired** (~95 commands total):

The four groups split across TUs under `examples/binapi2/fapi/async-demo-ui/rest/`:

```
rest/commands.hpp              # form_state, form_kind, cmd_ctx, rest_command,
                               # run_market_data / run_account / run_trade /
                               # run_convert free-fn templates, needs_* predicates
rest/commands.cpp              # needs_* bodies, reset_capture, CSV helper
rest/commands_market_data.cpp  # 39 endpoints
rest/commands_account.cpp      # 22 endpoints
rest/commands_trade.cpp        # 28 endpoints (incl. bespoke run_cancel_batch for
                               #                the non-generic batch-cancel path)
rest/commands_convert.cpp      # 3 endpoints  (dispatched via rest_pipeline())
views/rest_view.cpp            # aggregates the four registries, renders the menu
                               # with group headers, gates Run on credentials
```

The aggregated menu shows group headers (`── Market Data ──` / `── Account ──` /
`── Trade ──` / `── Convert ──`) as non-selectable rows; headers map to a sentinel
`cmd_of_menu[idx] == -1` and are skipped on Enter.

**Auth gating**: commands with `command_group != market_data` are gated on
`state.credentials_loaded`. When blocked, the Run event is a no-op and the
middle panel shows `⚠ run blocked: credentials required`.

**Form fields** now include: symbol, pair, limit, interval, period, orderId,
algoId, side, type, algoType, tif, quantity, price, leverage, marginType,
amount, delta_type, countdown, bool_flag (reused for the boolean toggle
commands), asset, ids (csv), startTime, endTime, downloadId, from_asset,
to_asset, from_amount, quoteId, convert orderId. Each input is `Maybe`-wrapped
so hidden inputs drop out of focus traversal.

**Acceptance**:

* All ~95 REST commands accessible from the REST tab with the right per-command
  input set.
* Public market-data commands runnable before credentials load; authenticated
  commands show the "credentials required" indicator and Enter is a no-op
  until credentials succeed.
* Switching commands keeps each command's last result and each field's last
  value sticky.

**Deferred**:

* Required-field validation (red border + disabled Run).
* Dropdowns (Toggle / Menu) for enum-typed fields (side, type, tif, margin
  type, delta_type, algo type) — currently free-text, parsed through
  `lib::parse_enum`.
* Per-group scroll markers / filter input if the flat menu grows unwieldy.
* Manual UI smoke testing — the binary builds clean and all 401 unit tests
  pass, but the FTXUI surface itself is not driven in CI; the user verifies
  by running `scripts/testnet/demo_ui.sh` (or equivalent) before declaring
  the step truly closed.

---

### Step 5 — all WS API requests

**Goal**: every command in `cmd_ws_api.cpp` works through the same form +
response pattern.

**Differences from REST**:

* Two underlying patterns: **public** (`exec_ws_public`) and
  **signed** (`exec_ws_signed`, which connects + logs on + executes).
* WS API responses have a `status` field plus an optional `result`. The
  response pane already handles this fine — we capture both.
* The **WS API connection is short-lived** (one connect+close per call).
  Step 5 keeps it that way for simplicity. Step 7 can introduce a
  persistent logged-in connection if it makes sense for trading.

**Files added**:

```
ws/
  commands_ws_api.cpp    # ~16 form factories
views/ws_view.cpp        # mirror of rest_view, populated from the WS list
```

`exec_ws_public` and `exec_ws_signed` already live in
`binapi2_demo_commands` (sink-parameterised). The WS commands reuse the
same `capture_sink` from step 1 — no new sink type needed.

**Acceptance**: every command in the WS API tab runs and shows results.

---

### Step 6 — all market streams

**Goal**: every subscription in `cmd_stream.cpp` selectable from the
Streams tab, with three view modes.

**View mode toggle**:

```
[ JSON list ] [ Compact table ] [ Counters only ]
```

* **JSON list**: same as step 3 — ring buffer of pretty-printed events.
* **Compact table**: typed access — render the latest N events as a
  `Table` (from `ftxui/dom/table.hpp`) with stream-specific columns
  (e.g. for `bookTicker`: time, symbol, bid, bidQty, ask, askQty). This
  needs **per-subscription column layouts** — a small visitor in
  `streams/table_renderers.cpp` switching on the subscription's event
  variant.
* **Counters only**: a single status line — `total events`, `events/s`
  (sliding-window EMA), `last event time`, `errors`. No per-event render at
  all, useful for high-rate streams like all-tickers.

The third mode is essentially **always-on**; it's just the counter line at
the top of the streams pane regardless of which mode is selected below.

**Recording**: **opt-in here.** Reuses the CLI's
`async_spdlog_stream_recorder` if `--record FILE` was passed at launch
(same flag as the CLI). The recorder runs as another spawned task; the
stream view shows a `recording → /path` indicator in the counter line.

**Files added**:

```
streams/
  commands_streams.cpp     # ~22 subscription form factories
  table_renderers.cpp      # per-subscription typed-table renderers
views/streams_view.cpp     # rewritten: full layout
```

**Acceptance**: every stream listed in `cmd_stream.cpp` is selectable, all
three view modes work, Stop returns to idle promptly, recording (if
enabled) writes a JSONL file matching the CLI's output byte-for-byte.

---

### Step 7 — full-featured order book + trades tape

**Goal**: turn the step-2 order book into something usable for actual
inspection. Split-screen with the **trades tape on the left** and the
**order book on the right** — so the eye reads the tape's most-recent
prints right up against the book they affected.

**Layout**:

```
┌──────────── Trades Tape ─────────────┬────────── Order Book ──────────┐
│   [symbol] [depth] [Start/Stop]      │                                │
│                                      │      ASK 29 412.20  ▌▌▌▌  0.512│
│  qty    price       side    time     │      ASK 29 412.10  ▌▌    0.221│
│  ────  ─────────  ────  ────────     │   ── spread 0.10 ── mid 29412.05│
│  0.412  29 412.10   B   09:01:23.412 │      BID 29 412.00  ▌     0.183│
│  0.221  29 412.20   S   09:01:23.401 │      BID 29 411.90  ▌▌    0.244│
│  0.500  29 412.20   S   09:01:23.392 │      …                         │
│  …                                   │                                │
│                              ←  ←  ← │                                │
└──────────────────────────────────────┴────────────────────────────────┘
```

**Why this orientation**:

* Tape rows are laid out with **price as the rightmost column on the left
  pane**, so the price lines up *visually adjacent* to the order book's
  price column on the right pane. Reading a tape row right-to-left the
  eye picks up: price → side → time → qty.
* New trades enter at the **top-right** of the tape (next to the book)
  and **scroll down/leftward** as more arrive — older entries drift away
  from the book. "Running right-to-left" in the sense that the freshest
  data hugs the book and history flows away from it.
* Buy = green row, sell = red row (using `m` flag from
  `aggregate_trade_event_t::market_maker_buyer`).

**Order book pane (right)**:

* Quantity bars scaled per-side using the max visible level.
* Top-of-book row bold + coloured (green for best bid, red for best ask).
* Spread + mid line between the two halves.
* Depth changes update at the natural feed rate.

**Tape pane (left)**:

* Bounded ring of the last ~50 trades, newest first.
* Each row: `qty │ price │ side │ time` — column order chosen so that
  `price` is hard against the right edge of the pane (i.e. closest to
  the book on the right).
* Tape updates per trade (~20–50 Hz on liquid pairs).

**Implementation**:

* Reuses `binapi2_demo_commands::exec_stream` from the shared library
  for the `aggTrade` subscription; the order book uses
  `local_order_book` directly (not via the executor).
* One Start button spawns *both* tasks; one Stop cancels both
  (`book->stop()` + flip the tape's `atomic<bool>` stop flag).
* The two captures (`book_capture` and `trades_tape_capture`) share
  the symbol field but otherwise live independently.

**Files added**:

```
book/
  trades_tape_capture.hpp    # newest-first bounded ring, mutex + atomic counters
views/orderbook_view.cpp     # rewritten: trades-tape pane on the LEFT,
                             # order book on the RIGHT, single Start/Stop
                             # spawns both tasks
```

**Acceptance**:

* One Start button brings up both panes; one Stop returns both to idle.
* Depth display and tape are consistent (a market-buy in the tape
  correlates with a quantity drop on the ASK side at that price level).
* The tape's price column is visually flush with the book's price column.
* No screen tearing under heavy load — PostEvent throttled to ≤30 Hz on
  both captures (independent throttles).

---

## 5. Open questions (resolve before each step)

1. **JSON tree click-to-expand vs. always-collapsed-by-default**: FTXUI
   `Collapsible` defaults to closed. For step 1 we leave them all closed
   (user expands manually). Reconsider after eyeballing real responses.

2. **Form value-binding for enums**: the CLI parses strings via
   `parse_enum<E>("BUY")`. For the UI, do we use a `Toggle` component with
   the enum's known string values, or an `Input` with parse-on-Run? Toggles
   are nicer but require listing the enum values per type. Default to
   **Input + parse-on-Run** in step 4 for speed; refactor to Toggles in
   a later cleanup pass if there's appetite.

3. **Multiple concurrent requests**: §3.4 assumes one in-flight REST call
   at a time (current_capture is a single atomic). If we want parallel calls
   from different tabs, swap the pointer for an id map. Likely **not needed
   until after step 7**.

4. **Live vs. testnet at runtime**: still a launch-time `--live` flag.
   In-UI toggle would mean tearing down and rebuilding the
   `futures_usdm_api`. Can be added in a polish pass — not on the critical
   path.

5. **Mouse support**: FTXUI's `App::TrackMouse(true)` enables it. Off by
   default (terminal selection wins). We'll likely **leave it off** — the
   demo is keyboard-driven and selection-from-terminal is more useful for
   copying JSON.

---

## 6. Risks / sharp edges

* **PostEvent rate**: under high-frequency streams, calling `PostEvent` per
  event will saturate FTXUI's event queue and starve input. **Throttle to
  ≤30 Hz** per capture — track last notify time, only post if elapsed.
  Needed from step 3 onwards.

* **Lock contention in render**: FTXUI repaints on every event. If the
  render path waits on a busy worker mutex, input lags. **Always copy
  out of the capture under the lock, then release before drawing.**

* **`spdlog` corrupting the screen**: any binapi internals that log via
  spdlog will scribble onto the TUI. Step 0 already redirects the default
  logger to a null sink. If step 4+ surfaces specific log messages we want
  to see, route them through `app_state::status_message` instead.

* **Cobalt + cancellation**: cobalt's cancellation story is rough around
  the edges. Step 2 (order book) sidesteps it via `local_order_book::stop()`.
  Step 3 (streams) sidesteps it via an `atomic<bool> stop` checked between
  events. We never depend on the asio cancellation slot machinery — if
  this becomes limiting, revisit in a future refactor.

* **GCC 15 + `cobalt::join`**: known to ICE in nested joins (per
  `async-recorder` notes). The UI never uses `cobalt::join` — every
  background task is `cobalt::spawn`-and-detach. Sidestepped by design.

* **FTXUI version drift**: pinned to v6.1.9-114 via the `deps/FTXUI`
  submodule. No floating-version risk unless the submodule is updated.

---

## 7. File layout (target — after step 7)

### Shared library (created in step 0.5)

```
libs/binapi2_demo_commands/
├── CMakeLists.txt
└── include/binapi2/demo_commands/
    ├── result_sink.hpp        # abstract sink: 5 virtual fns
    ├── spdlog_sink.hpp        # CLI implementation
    ├── exec.hpp               # exec_market_data / exec_account / exec_trade
    │                          #   / exec_ws_public / exec_ws_signed / exec_stream
    ├── enum_parse.hpp         # parse_enum<E>(string_view)
    ├── opts.hpp               # symbol_opts, kline_opts, order_opts, …
    └── builders.hpp           # make_*_request(opts) -> Request
```

Consumers: `binapi2-fapi-async-demo-cli`, `binapi2-fapi-async-demo-ui`.

### Per-binary code (UI)

```
examples/binapi2/fapi/async-demo-ui/
├── CMakeLists.txt
├── main.cpp
├── app_state.hpp
├── worker.hpp / worker.cpp
├── util/
│   ├── capture_sink.hpp        # implements demo::result_sink → request_capture
│   ├── request_capture.hpp     # struct from §3.3
│   ├── json_tree.hpp           # walks glz::json_t into FTXUI Collapsibles
│   └── throttle.hpp            # PostEvent throttle helper
├── rest/
│   ├── commands.hpp            # ui_command + registry
│   ├── commands_market_data.cpp
│   ├── commands_account.cpp
│   ├── commands_trade.cpp
│   └── commands_convert.cpp
├── ws/
│   └── commands_ws_api.cpp
├── streams/
│   ├── stream_capture.hpp
│   ├── commands_streams.cpp
│   └── table_renderers.cpp
├── book/
│   ├── book_capture.hpp
│   └── trades_tape_capture.hpp
└── views/
    ├── views.hpp
    ├── status_bar.cpp
    ├── rest_view.cpp
    ├── ws_view.cpp
    ├── streams_view.cpp
    └── orderbook_view.cpp        # tape (left) + book (right)
```

Note that `parse_enum.hpp`, `rest_executor.hpp`, `ws_executor.hpp`,
`stream_runner.hpp`, and `book_runner.hpp` from earlier sketches all
live in the shared library now — the UI only needs the capture sink and
the surface code.

### Per-binary code (CLI, after step 0.5 refactor)

The CLI keeps its existing layout, with `common.hpp` shrunk to spdlog
plumbing + config helpers and each `cmd_*.cpp` calling shared opts +
builders + executors instead of inline-rolling them.

Each step adds files in increments — never deletes them (except as
noted in step 4 collapsing `ping_command.cpp` into the registry).
