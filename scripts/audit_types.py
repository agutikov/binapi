#!/usr/bin/env python3
"""Audit script that builds a matching table of all binapi2 data types.

Scans C++ headers, API docs, JSON examples, and data_types.md to produce
a cross-reference table showing coverage gaps.
"""

import json
import os
import re
import signal
import sys
import time
from pathlib import Path

# Graceful Ctrl+C
def _sigint_handler(sig, frame):
    print("\n\nInterrupted.", file=sys.stderr)
    sys.exit(130)

signal.signal(signal.SIGINT, _sigint_handler)

def log(msg):
    print(f"[audit] {msg}", file=sys.stderr, flush=True)

REPO_ROOT = Path(__file__).resolve().parent.parent

TYPES_DIR = REPO_ROOT / "include" / "binapi2" / "fapi" / "types"
DOCS_DIR = REPO_ROOT / "docs" / "api" / "md"
RESPONSES_DIR = REPO_ROOT / "compose" / "postman-mock" / "responses"
DATA_TYPES_MD = REPO_ROOT / "docs" / "binapi2" / "data_types.md"


def find_headers():
    """Return sorted list of .hpp files directly in the types directory."""
    headers = []
    for f in sorted(TYPES_DIR.iterdir()):
        if f.suffix == ".hpp" and f.is_file():
            headers.append(f)
    return headers


def parse_structs(header_path):
    """Parse a single header file and extract struct information.

    Returns a list of dicts with keys: name, header, fields, doc_path.
    """
    text = header_path.read_text()
    lines = text.split("\n")

    structs = []
    pending_doc = None
    i = 0
    iterations = 0
    while i < len(lines):
        iterations += 1
        if iterations > len(lines) * 3:
            log(f"    WARNING: possible infinite loop at line {i}, breaking")
            break
        line = lines[i]

        # Track doc comments
        doc_match = re.match(r"\s*//\s*doc:\s*(\S+)", line)
        if doc_match:
            pending_doc = doc_match.group(1)
            i += 1
            continue

        # Match struct declarations - both multi-line and single-line
        # Multi-line: struct name_t { ... };  or  struct name_t : base_t { ... };
        # Single-line: struct name_t { fields; };
        struct_match = re.match(
            r"\s*struct\s+(\w+)"
            r"(?:\s*:\s*\w+)?"     # optional base class
            r"\s*\{?"              # optional opening brace on same line
            , line
        )
        # Skip template structs (like websocket_api_response_t<T>)
        if struct_match and "<" not in line:
            # But also skip "struct glz::meta" specializations
            if "glz::meta" in line:
                pending_doc = None
                i += 1
                continue

            struct_name = struct_match.group(1)

            # Find the struct body
            # Check if it's a single-line struct: struct name { ... };
            single_line = re.match(
                r"\s*struct\s+\w+(?:\s*:\s*\w+)?\s*\{(.*?)\}\s*;",
                line
            )

            if single_line:
                body = single_line.group(1)
                field_count = _count_fields_in_body(body)
                i += 1  # advance past this line
            elif "{" in line:
                body_lines = []
                brace_depth = line.count("{") - line.count("}")
                i += 1
                while i < len(lines) and brace_depth > 0:
                    brace_depth += lines[i].count("{") - lines[i].count("}")
                    if brace_depth > 0:
                        body_lines.append(lines[i])
                    i += 1
                field_count = _count_fields_in_lines(body_lines)
                # i already past the closing brace
            else:
                i += 1
                if i < len(lines) and "{" in lines[i]:
                    body_lines = []
                    brace_depth = lines[i].count("{") - lines[i].count("}")
                    i += 1
                    while i < len(lines) and brace_depth > 0:
                        brace_depth += lines[i].count("{") - lines[i].count("}")
                        if brace_depth > 0:
                            body_lines.append(lines[i])
                        i += 1
                    field_count = _count_fields_in_lines(body_lines)
                else:
                    field_count = 0
                    # i already advanced by 1

            structs.append({
                "name": struct_name,
                "header": str(header_path.relative_to(TYPES_DIR)),
                "fields": field_count,
                "doc_path": pending_doc,
            })
            pending_doc = None
            continue

        # Reset doc if we hit a non-doc, non-struct, non-blank line
        if line.strip() and not line.strip().startswith("//"):
            pending_doc = None

        i += 1

    return structs


def _count_fields_in_body(body_text):
    """Count fields in a single-line struct body."""
    # Split by ; and count non-empty declarations
    parts = [p.strip() for p in body_text.split(";") if p.strip()]
    count = 0
    for part in parts:
        if _is_field_declaration(part):
            count += 1
    return count


def _count_fields_in_lines(body_lines):
    """Count member field declarations in multi-line struct body lines."""
    count = 0
    nested_depth = 0
    for line in body_lines:
        stripped = line.strip()
        if not stripped:
            continue

        # Track nested braces for nested structs/lambdas
        # Skip lines inside nested struct/lambda bodies
        open_b = stripped.count("{")
        close_b = stripped.count("}")

        # If we're inside a nested scope (not the main struct), skip
        if nested_depth > 0:
            nested_depth += open_b - close_b
            continue

        # A line that opens a new nested scope (e.g., lambda, nested struct)
        # but also declares something -- handle carefully
        if "struct " in stripped and not stripped.startswith("std::"):
            nested_depth += open_b - close_b
            continue

        if _is_field_line(stripped):
            count += 1

        # Update nesting after checking (for lines that open lambdas etc.)
        # Actually lambdas appear in glz::meta, not in struct bodies here
        nested_depth += open_b - close_b
        if nested_depth < 0:
            nested_depth = 0

    return count


def _is_field_line(line):
    """Check if a line is a member field declaration."""
    # Skip comments
    if line.startswith("//") or line.startswith("///"):
        return False
    # Skip using declarations
    if line.startswith("using "):
        return False
    # Skip static constexpr
    if line.startswith("static "):
        return False
    # Skip function declarations (have parentheses not inside template args)
    # but allow lines with {} for value initialization
    # Skip empty lines / just braces
    if line in ("{", "}", "};", "{};"):
        return False

    # A field declaration typically looks like:
    #   type name{};
    #   type name{value};
    #   type name;
    #   std::optional<type> name{};
    # It must contain a semicolon
    if ";" not in line:
        return False

    # Skip lines that look like function declarations (have parens before ;)
    # but not lines like std::optional<foo> bar{};
    # Remove template angle brackets content to avoid confusion
    cleaned = _remove_templates(line)
    # If there's a ( before ; it's likely a function
    semi_pos = cleaned.find(";")
    paren_pos = cleaned.find("(")
    if paren_pos != -1 and paren_pos < semi_pos:
        return False

    return True


def _is_field_declaration(part):
    """Check if a single declaration part (between semicolons) is a field."""
    part = part.strip()
    if not part:
        return False
    if part.startswith("using "):
        return False
    if part.startswith("static "):
        return False
    if part.startswith("//"):
        return False
    # Should have at least a type and a name
    # Remove template stuff
    cleaned = _remove_templates(part)
    if "(" in cleaned:
        return False
    # Must have at least two tokens (type + name)
    tokens = cleaned.split()
    return len(tokens) >= 2


def _remove_templates(text):
    """Remove template angle bracket contents for simpler parsing."""
    result = []
    depth = 0
    for ch in text:
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth = max(0, depth - 1)
        elif depth == 0:
            result.append(ch)
    return "".join(result)


def find_api_docs():
    """Walk the docs/api/md directory and return a set of all .md file paths
    (relative to repo root, prefixed with /docs/api/md/)."""
    docs = {}
    if not DOCS_DIR.exists():
        log("  docs/api/md not found, skipping")
        return docs
    count = 0
    for md_file in DOCS_DIR.rglob("*.md"):
        rel = "/" + str(md_file.relative_to(REPO_ROOT))
        docs[rel] = md_file
        count += 1
    log(f"  walked {count} markdown files")
    return docs


def load_json_examples():
    """Load JSON example files from the responses directory.

    Returns a dict: filename_stem -> (filepath, key_count).
    """
    examples = {}
    if not RESPONSES_DIR.exists():
        return examples
    for f in sorted(RESPONSES_DIR.iterdir()):
        if f.suffix == ".json" and not f.name.endswith(".schema.json"):
            try:
                data = json.loads(f.read_text())
                if isinstance(data, dict):
                    key_count = len(data)
                elif isinstance(data, list) and data and isinstance(data[0], dict):
                    key_count = len(data[0])
                else:
                    key_count = 0
                examples[f.stem] = (f.name, key_count)
            except (json.JSONDecodeError, IndexError):
                examples[f.stem] = (f.name, 0)
    return examples


def load_data_types_md():
    """Load data_types.md and return set of type names mentioned."""
    mentioned = set()
    if not DATA_TYPES_MD.exists():
        return mentioned
    text = DATA_TYPES_MD.read_text()
    # Find all backtick-quoted type names
    for m in re.finditer(r"`(\w+_t)`", text):
        mentioned.add(m.group(1))
    # Also find subscription types and other non-_t types
    for m in re.finditer(r"`(\w+(?:_subscription|_response|_request|_event|_entry|_data))`", text):
        mentioned.add(m.group(1))
    return mentioned


def match_json_example(struct_name, json_examples):
    """Try to match a C++ type name to a JSON example file by name similarity."""
    # Strip common suffixes to get a base name
    base = struct_name
    for suffix in ("_response_t", "_request_t", "_entry_t", "_t"):
        if base.endswith(suffix):
            base = base[: -len(suffix)]
            break

    # Direct match
    if base in json_examples:
        return json_examples[base]

    # Try common transformations
    # e.g., order_book -> depth, futures_account_balance -> balance
    name_map = {
        "order_book": "depth",
        "futures_account_balance": "balance",
        "account_information": "account",
        "recent_trade": "trades",
        "book_ticker": "ticker_book",
        "price_ticker": "ticker_price",
        "mark_price": "premium_index",
        "listen_key": "listen_key",
        "server_time": "server_time",
        "exchange_info": "exchange_info",
        "kline": "klines",
        "funding_rate_history": "funding_rate",
        "position_risk": "position_risk",
        "order": "order",
        "open_interest": "open_orders",
    }

    if base in name_map and name_map[base] in json_examples:
        return json_examples[name_map[base]]

    # Fuzzy: check if any json stem is contained in the base or vice versa
    for stem, val in json_examples.items():
        stem_normalized = stem.replace("_", "")
        base_normalized = base.replace("_", "")
        if stem_normalized in base_normalized or base_normalized in stem_normalized:
            return val

    return None


def main():
    t0 = time.monotonic()

    log("scanning type headers...")
    headers = find_headers()
    log(f"  found {len(headers)} headers ({time.monotonic()-t0:.1f}s)")

    log("scanning API docs...")
    api_docs = find_api_docs()
    log(f"  found {len(api_docs)} docs ({time.monotonic()-t0:.1f}s)")

    log("loading JSON examples...")
    json_examples = load_json_examples()
    log(f"  found {len(json_examples)} examples ({time.monotonic()-t0:.1f}s)")

    log("loading data_types.md...")
    data_types_mentioned = load_data_types_md()
    log(f"  found {len(data_types_mentioned)} type mentions ({time.monotonic()-t0:.1f}s)")

    log("parsing structs from headers...")
    all_structs = []
    for header in headers:
        t1 = time.monotonic()
        log(f"  parsing {header.name} ({header.stat().st_size} bytes)...")
        structs = parse_structs(header)
        log(f"    -> {len(structs)} structs ({time.monotonic()-t1:.1f}s)")
        all_structs.extend(structs)

    log(f"total: {len(all_structs)} structs ({time.monotonic()-t0:.1f}s)")

    log("matching types to docs/json/data_types.md...")
    # Sort by header file, then by struct name
    all_structs.sort(key=lambda s: (s["header"], s["name"]))

    # Build table
    col_type = "C++ Type"
    col_header = "Header"
    col_fields = "Fields"
    col_doc = "API Doc"
    col_json = "JSON Example"
    col_keys = "JSON Keys"
    col_dt = "data_types.md"

    rows = []
    matched_docs = 0
    matched_json = 0

    for s in all_structs:
        name = s["name"]
        header = s["header"]
        fields = s["fields"]

        # API doc match
        doc_path = s["doc_path"]
        if doc_path and doc_path in api_docs:
            # Show just the filename
            doc_display = Path(doc_path).name
            matched_docs += 1
        elif doc_path:
            # Doc comment exists but file not found
            doc_display = Path(doc_path).name + " (!)"
            matched_docs += 1
        else:
            doc_display = "\u2014"

        # JSON example match
        json_match = match_json_example(name, json_examples)
        if json_match:
            json_display = json_match[0]
            json_keys = str(json_match[1])
            matched_json += 1
        else:
            json_display = "\u2014"
            json_keys = "\u2014"

        # data_types.md mention
        dt_mention = "yes" if name in data_types_mentioned else "no"

        rows.append((name, header, str(fields), doc_display, json_display, json_keys, dt_mention))

    # Calculate column widths
    all_rows = [(col_type, col_header, col_fields, col_doc, col_json, col_keys, col_dt)] + rows
    widths = [0] * 7
    for row in all_rows:
        for i, val in enumerate(row):
            widths[i] = max(widths[i], len(val))

    def fmt_row(row):
        parts = []
        for i, val in enumerate(row):
            if i == 2 or i == 5:  # Fields, JSON Keys - right align
                parts.append(val.rjust(widths[i]))
            else:
                parts.append(val.ljust(widths[i]))
        return "  ".join(parts)

    log(f"building table ({time.monotonic()-t0:.1f}s)")

    # Print table
    header_row = fmt_row(all_rows[0])
    separator = "  ".join("-" * w for w in widths)

    print(header_row)
    print(separator)
    for row in rows:
        print(fmt_row(row))

    # Summary
    total = len(all_structs)
    print()
    print(f"Total types: {total}")
    print(f"Matched to API docs: {matched_docs}")
    print(f"Matched to JSON examples: {matched_json}")
    print(f"Mentioned in data_types.md: {sum(1 for r in rows if r[6] == 'yes')}")


if __name__ == "__main__":
    main()
