# `binapi2-fapi-async-demo-ui` — interactive TUI

FTXUI-based demonstration client for the `binapi2` USD-M Futures library.
Same coverage as the CLI (`binapi2-fapi-async-demo-cli`) but as four tabs
in an interactive terminal interface: **REST**, **WS API**, **Streams**,
**Order Book**.

Binary: `_build/examples/binapi2/fapi/async-demo-ui/binapi2-fapi-async-demo-ui`

```
$ binapi2-fapi-async-demo-ui --help
binapi2-fapi-async-demo-ui — FTXUI demonstration client
  -l,--live,--prod        Use production endpoints (default: testnet)
  -L,--log-file PATH      Log to file (TUI owns stdout, so no stderr logging)
  -F,--file-loglevel LVL  Log level for -L (trace|debug|info|warn|error|…)
```

## Layout

```
┌ TESTNET  creds: ok  jobs: 0  │  <status msg>                ┐  ← status bar
│  [REST] [WS API] [Streams] [Order Book]                     │  ← tab toggle
├──────────────────────────────────────────────────────────────┤
│ ┌ menu ─┬ form ──┬ rr_pane / event list / book+tape ──────┐ │
│ │ …     │  …     │                                        │ │
│ └───────┴────────┴────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│  ↑↓ select   Enter run/toggle   → form   Tab cycle   q quit │  ← context-aware keybar
└──────────────────────────────────────────────────────────────┘
```

* **Status bar** (`views/status_bar.cpp`) — TESTNET/LIVE badge, credential
  state, in-flight job count, last status message. Left-aligned, gray
  background, non-interactive (not in the focus chain).
* **Tab toggle** — FTXUI `Toggle` driving `Container::Tab`.
* **Tab content** — one of `make_rest_view`, `make_ws_view`,
  `make_streams_view`, `make_orderbook_view`.
* **Keybar** — `Renderer` in `main.cpp`. Each frame reads
  `active_tab.hints()` and joins the returned chips. The hints function
  probes which child has focus (`Focused()`) and returns zone-specific
  keys. Global `Tab` / `q` are caught at the outer layout.

## File layout

```
examples/binapi2/fapi/async-demo-ui/
├── main.cpp                       # CLI parsing, spdlog routing, screen.Loop
├── app_state.hpp                  # atomics + mutex-guarded strings shared main↔worker
├── worker.hpp/.cpp                # io_context + thread; acquire_rest_client; notify_ui
├── rest/
│   ├── commands.hpp/.cpp          # shared form_state / form_kind / cmd_ctx / rest_command
│   ├── commands_market_data.cpp   # 39 market-data entries
│   ├── commands_account.cpp       # 21 account entries (auth)
│   ├── commands_trade.cpp         # 29 trade entries (auth)
│   └── commands_convert.cpp       # 3 convert entries (auth)
├── ws/
│   ├── commands.hpp               # declares ws_commands() + ws_logon_coro
│   └── commands.cpp               # 16 WS API entries (reuses rest:: types)
├── streams/
│   ├── commands.hpp/.cpp          # stream_form_state / form_kind + 21 subscriptions
│   ├── stream_capture.hpp         # per-stream ring buffer + atomic stop/running
│   └── stream_capture_sink.hpp    # result_sink → push into ring; throttle notify
├── book/
│   ├── book_capture.hpp           # snapshot + book_ptr for Stop
│   └── trades_tape_capture.hpp    # bounded trade ring for the tape pane
├── util/
│   ├── capture_sink.hpp           # REST/WS result_sink → request_capture
│   ├── request_capture.hpp        # capture_side { raw, pretty_json, parsed_json }
│   ├── wrap_text.hpp              # word wrap with force-break for long tokens
│   ├── virtual_scroll.hpp         # scroll_model + virtual_scroll_render + scrollbar
│   └── json_tree.hpp              # glz::generic → vector<Element> tree rows
└── views/
    ├── views.hpp                  # tab_view + key_chip + make_* factory decls
    ├── status_bar.cpp
    ├── response_pane.hpp          # shared Raw/JSON/Tree sub-tab pane (REST + WS)
    ├── rest_view.cpp              # REST tab (collapsible groups, ~92 cmds)
    ├── ws_view.cpp                # WS API tab (flat list, 16 cmds)
    ├── streams_view.cpp           # Streams tab (21 subs, virtualized event list)
    └── orderbook_view.cpp         # Order Book tab (tape | book split)
```

## Shared infrastructure

### `tab_view` + `key_chip` (`views/views.hpp`)

Each `make_*_view` returns:

```cpp
struct tab_view {
    ftxui::Component component;
    std::function<std::vector<ftxui::Element>()> hints;
};
```

`main.cpp` aggregates all four and queries the active tab's `hints()`
each frame. Chips are built with the shared helper:

```cpp
inline Element key_chip(const char* keys, const char* desc);
```

— bold yellow keys + plain description, drawn on a gray-dark background.

### Virtual scroll (`util/virtual_scroll.hpp`)

Used by every scrollable content pane (REST/WS response tabs, the
Streams event list). Not using `yframe` — that renders the full child
into a virtual screen every frame then clips, O(total_rows). Instead:

1. Caller pre-builds a `std::vector<Element>` of rows.
2. `virtual_scroll_render(rows, model)` slices
   `rows[scroll_top .. scroll_top + viewport_h]` and emits that as an
   `hbox(vbox(slice) | flex, scrollbar_column(…))`.
3. `reflect(probe_box)` on the outer hbox captures viewport size for
   the **next** frame. First frame uses a tiny fallback (1×40) so it
   can't overflow into neighbouring layout.

```cpp
struct scroll_model {
    int scroll_top = 0;
    int viewport_h = 0;
    int viewport_w = 0;   // content area width, excludes scrollbar column
    int total_rows = 0;
};
```

PageUp/PageDown = `viewport_h / 2`. Mouse wheel (`Mouse::WheelUp/Down`,
step 3) is gated on `probe_box->Contain(mouse.x, mouse.y)` so the menu
still gets wheel events when the cursor is over it.

### Word wrap (`util/wrap_text.hpp`)

`wrap_lines(content, width)` produces a flat `vector<string>`:
* `\n` is a hard break (preserved one-for-one).
* Within each logical line, greedy pack up to `width` characters,
  breaking at the last whitespace in the current window.
* Tokens longer than `width` (e.g. long URLs, base64 blobs) are
  force-broken at exactly `width` chars.

`wrap_paragraph(content, width)` wraps the result into a
`vbox({ text(line), text(line), … })` — a plain vbox of text rows,
~10× cheaper to render than `paragraph()` (which makes one flexbox
per line).

### Response pane (`views/response_pane.hpp`)

`make_request_response_pane(get_cap)` returns a vbox split 50/50:
**REQUEST** (top) and **RESPONSE** (bottom). Each half has three
sub-tabs — **Raw** (HTTP message from `cfg.logger`), **JSON** (pretty
`glz::write` of the typed struct), **Tree** (`render_json_tree_rows`
over the parsed `glz::generic`). Each sub-tab has its own
`scroll_model`; all three in a side pane share one `probe_box` so
first-render sizes the JSON/Tree tab from the Raw tab's reflected
box.

### Request / stream captures

Two independent paths:

```cpp
struct request_capture {      // util/request_capture.hpp
    enum state_t { idle, running, done, failed };
    std::atomic<state_t> state;
    capture_side request;     // raw + pretty_json + parsed glz::generic
    capture_side response;
    std::string  error_message;
    int http_status, binance_code;
    std::mutex   mtx;
};
```

```cpp
struct stream_capture {        // streams/stream_capture.hpp
    std::atomic<bool> stop{false}, running{false};
    std::atomic<uint64_t> total_events{0}, errors{0};
    std::deque<std::string> ring;   // last max_ring = 200 events
    static constexpr std::size_t max_ring = 200;
    std::mutex mtx;
};
```

`capture_sink` (`util/capture_sink.hpp`) implements `result_sink` for
one-shot REST/WS calls — writes pretty JSON + raw HTTP into the
capture, flips `state`, toggles `active_jobs`. `stream_capture_sink`
does the streaming variant — appends to the ring, throttles
`notify_ui` to 30 Hz.

## Tab anatomy

### REST tab — `views/rest_view.cpp`

```
┌─ Menu ──────────┬─ Form + info ───┬─ rr_pane ───┐
│ ▼ Market Data   │ ping            │ REQUEST     │
│   ping          │ Test API …      │ [Raw][JSON] │
│   server-time   │ state: idle     │ …           │
│   …             │ info: (none)    ├─────────────┤
│ ▶ Account       │                 │ RESPONSE    │
│ ▶ Trade         │                 │ [Raw][JSON] │
│ ▶ Convert       │                 │ …           │
└─────────────────┴─────────────────┴─────────────┘
```

* **Registry** — `rest::market_data_commands()`, `account_commands()`,
  `trade_commands()`, `convert_commands()`. Each entry:
  ```cpp
  struct rest_command {
      const char* name;
      const char* description;
      command_group group;
      form_kind form;
      void (*run)(const cmd_ctx&);
  };
  ```
* **Menu** — flat aggregation of the four group spans, with
  `▼`/`▶` collapsible headers. `visible_titles` + `visible_map`
  rebuild on each toggle, preserving selection by entry identity.
  Scrollable via `| vscroll_indicator | yframe | flex`.
* **Form** — `Container::Vertical` of Maybe-gated `Input` widgets
  (~30 total, one per field). Visibility predicates in
  `rest::needs_symbol(fk)`, `needs_pair(fk)`, …
* **Auth gating** — `rest::requires_auth(group)` drives a "⚠ run
  blocked: credentials required" banner; Enter is a no-op until
  `state.credentials_loaded`.
* **Per-command capture** — `vector<shared_ptr<request_capture>>`,
  one entry per aggregated command. Switching commands shows that
  command's last result.

### WS API tab — `views/ws_view.cpp`

Same shape as REST, simpler:

* **Registry** — `ws::ws_commands()` returns a flat span of 16 entries
  reusing `rest::rest_command` / `form_kind` / `cmd_ctx`.
* Two `command_group` values: `ws_public` (no auth) and `ws_signed`
  (auth required). `rest::requires_auth` handles gating uniformly.
* **Bespoke**: `ws::ws_logon_coro` — connect + `async_session_logon`,
  no `exec_ws_*` call. `cmd_ws_logon` spawns this directly rather than
  via the templated dispatcher.
* Public tickers (`ws-book-ticker`, `ws-price-ticker`) dispatch to the
  plural endpoint when the symbol field is blank — mirrors the CLI.
* No collapsible groups (one group only → flat list).

### Streams tab — `views/streams_view.cpp`

```
┌─ Menu ──────────┬─ Form + status ───┬─ Event list ─┐
│ bookTicker      │ bookTicker        │  {           │
│ aggTrade        │ symbol: BTCUSDT   │    "e": …    │
│ markPrice       │ RUNNING events:12 │    …         │
│ …               │                   │  }           │
│ !bookTicker     │                   │  (next evt)  │
│ !ticker         │                   │  …           │
│ …               │                   │              │
└─────────────────┴───────────────────┴──────────────┘
```

* **Registry** — `streams::stream_commands()`. Own `form_state` /
  `form_kind` (not sharing `rest::`) because the action is start/stop,
  not single-shot run:
  ```cpp
  struct stream_command {
      const char* name;
      const char* description;
      form_kind form;                  // no_args | symbol | kline |
                                       // pair_kline | levels | speed
      void (*start)(const start_ctx&);
  };
  ```
* **Enter** toggles start/stop: spawns via `start(ctx)` if not
  running, flips `cap->stop = true` otherwise.
* **Per-stream scroll** — one `scroll_model` + cached wrapped-rows
  per subscription. Rebuilt when `cap->total_events` advances
  (monotonic stamp) or the pane resizes.
* **Form fields** — `symbol`, `pair`, `interval`, `levels` (5/10/20),
  `speed` (100ms/250ms).

### Order Book tab — `views/orderbook_view.cpp`

```
┌─ symbol [BTCUSDT] depth [1000] [Start] [Stop] ────┐
├───────────────── Trades Tape ─┬── Order Book ────┤
│ qty   side  time      price   │ ASK 29412.20     │
│ 0.41  B     09:01:23  29412.10│ …                │
│ …                             │ ── spread / mid ──│
│                               │ BID 29412.00     │
│                               │ …                │
└───────────────────────────────┴──────────────────┘
```

* Single controls row (symbol, depth, Start, Stop) above a split body.
* **Start** spawns two coroutines: `book_coro` (local order book) and
  `tape_coro` (aggregate-trade subscription).
* **Stop** calls `book->stop()` **and** flips `tape_cap->stop`.
* Tape `tape_sink` re-parses each pretty-JSON event back into
  `aggregate_trade_stream_event_t` to pull typed fields for display.
* Buy/sell colouring from `is_buyer_maker` (seller-aggressor → red,
  buyer-aggressor → green).
* Price is the rightmost tape column so it aligns visually with the
  book's price column across the vertical separator.

## Adding new things

### A new REST command

Add one free function + one row in a `rest/commands_*.cpp`:

```cpp
void cmd_my_thing(const R::cmd_ctx& c)
{
    types::my_request_t req;
    req.symbol = c.form.symbol;
    req.limit  = R::parse_optional_int(c.form.limit);
    run_market(c, std::move(req));   // or run_acct / run_trd / run_conv
}
// ...
{ "my-thing", "Description",
  command_group::market_data, form_kind::symbol_limit, &cmd_my_thing },
```

If the command needs a field that doesn't exist in `form_state`: add
the string to `rest::form_state`, extend the `form_kind` enum, add
`needs_<field>(form_kind)` to `rest/commands.cpp`, then wire an
`Input` in `rest_view.cpp` + a `labeled` render row. One-off shapes
(like basis's `pair + period`) are fine — the `form_kind` enum
already has many per-shape variants.

### A new WS API command

Same pattern in `ws/commands.cpp`. Public → `run_pub`, signed →
`run_sig`. The entry's `command_group` controls auth gating:

```cpp
{ "ws-my-call", "Description",
  R::command_group::ws_signed, R::form_kind::symbol, &cmd_ws_my_call },
```

### A new stream subscription

Add a `start_*` free function in `streams/commands.cpp`:

```cpp
void start_my_stream(const start_ctx& c)
{
    types::my_subscription s;
    s.symbol = c.form.symbol;
    spawn_stream(c, std::move(s));     // templated free-fn coroutine
}
// ...
{ "myStream", "Description", form_kind::symbol, &start_my_stream },
```

If the subscription has a field not yet in `streams::form_state`: add
the field, extend `form_kind`, add a `needs_*` predicate, wire the
input + render row in `streams_view.cpp`.

## Conventions

### Coroutine lifetime

Cobalt task bodies are **free functions** (or free-function template
instantiations), never local lambdas. A local lambda's captures are
destroyed when the spawning scope ends, but the coroutine frame keeps
pointers back into them — use-after-free on the next resumption. See
`feedback_cobalt_lambda_lifetime.md`.

Applied consistently: `rest::run_market_data<Req>`,
`ws::run_ws_pub<Req>`, `streams::run_stream_sub<Sub>`, etc. —
all templates instantiated once per request/subscription type.

### FTXUI model lifetime

FTXUI `Menu` / `Toggle` / `Input` widgets hold **raw pointers** into
caller-owned models — they never copy the container. Every backing
model must live in a `shared_ptr` explicitly captured by the outer
`Renderer` / `CatchEvent` lambda. If you pass `&agg->titles` to
`Menu`, make sure `agg` is captured.

First render crash with `std::bad_alloc` in
`ConstStringListRef::operator[]` is the signature of a dangling
Menu model pointer. See `feedback_ftxui_model_lifetime.md`.

### Focus chain

Each tab composes widgets in a `Container::Horizontal` of three
zones: **menu | form | content pane**. The form is itself a
`Container::Vertical` so:

* →/← walks between **zones** (menu / form / content).
* ↑/↓ walks inputs **within** the form.
* `Maybe`-wrapped inputs drop out of focus traversal when hidden.

FTXUI's `Menu` eats PageUp/PageDown and mouse wheel when focused
(for its own scrolling). To scroll the content pane you have to first
→ into it; the pane's `CatchEvent` handles PageUp/Down and wheel from
there.

### Notify throttle

All sinks that fire from the worker thread (capture_sink,
stream_capture_sink, book/tape snapshot callbacks) throttle their
`worker.notify_ui()` calls to ~30 Hz. Without this FTXUI's event
queue can be drowned by high-rate streams (e.g. all-tickers).

## Known limitations

* Enum fields (side, type, tif, margin_type, algo_type) are free-text
  `Input`s parsed through `lib::parse_enum` — typos produce an API
  error at run time. A `Toggle`/`Menu` dropdown would be a nice
  addition.
* No required-field validation yet (red border / disabled Run).
* No filter input for the REST menu — at ~92 entries the menu is long
  but scrolling is fine.
* Streams tab shows JSON list only; compact-table view + events/s EMA
  + recording are unimplemented.
* Order-book quantity bars use `std::stod` on decimals — display-only
  precision.
* No separate scrollbar on the tape pane (ring capped at 100, the
  `yframe` alone suffices).
* Manual UI smoke testing is not driven in CI; the binary builds +
  links + launches, but verification of interaction (menu nav, scroll,
  wheel, tab switch, start/stop on streams/book) has to be done by a
  human at a terminal.
