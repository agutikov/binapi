#!/usr/bin/env python3
"""Merge mock response bodies into a Postman collection.

Reads the original Postman collection and a directory of JSON response files,
then produces an enriched collection where matched endpoints have example
responses injected.  The mapping from response file to endpoint is defined
by a JSON mapping file.

Usage:
    merge_responses.py --collection <col.json> \
                       --responses <dir>       \
                       --mapping <map.json>    \
                       --output <enriched.json>
"""

import argparse
import json
import os
import sys


def load_json(path):
    with open(path) as f:
        return json.load(f)


def make_response_entry(name, request_obj, body_str):
    """Build a Postman v2.1 response entry."""
    return {
        "name": name,
        "originalRequest": request_obj,
        "status": "OK",
        "code": 200,
        "_postman_previewlanguage": "json",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": body_str,
    }


def endpoint_path(item):
    """Extract '/fapi/v1/order' style path from a Postman item."""
    try:
        parts = item["request"]["url"]["path"]
        return "/" + "/".join(parts)
    except (KeyError, TypeError):
        return None


def endpoint_method(item):
    try:
        return item["request"]["method"]
    except (KeyError, TypeError):
        return None


def inject_responses(items, response_map):
    """Recursively walk the item tree and inject responses where mapped."""
    injected = 0
    for item in items:
        # Folder — recurse.
        if "item" in item and item["item"]:
            injected += inject_responses(item["item"], response_map)
            continue

        path = endpoint_path(item)
        method = endpoint_method(item)
        if not path or not method:
            continue

        key = f"{method} {path}"
        if key not in response_map:
            continue

        body_str = response_map[key]
        resp = make_response_entry("200 OK", item.get("request", {}), body_str)
        item.setdefault("response", [])
        item["response"].append(resp)
        injected += 1

    return injected


def build_response_map(mapping, responses_dir):
    """Build {method+path -> body_string} from the mapping config."""
    result = {}
    for entry in mapping:
        filepath = os.path.join(responses_dir, entry["file"])
        if not os.path.isfile(filepath):
            print(f"warning: response file not found: {filepath}", file=sys.stderr)
            continue

        body = load_json(filepath)
        body_str = json.dumps(body, separators=(",", ":"))

        for ep in entry["endpoints"]:
            method = ep["method"]
            path = ep["path"]
            key = f"{method} {path}"
            result[key] = body_str

    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--collection", required=True, help="Input Postman collection JSON")
    parser.add_argument("--responses", required=True, help="Directory with response JSON files")
    parser.add_argument("--mapping", required=True, help="Mapping JSON file")
    parser.add_argument("--output", required=True, help="Output enriched collection JSON")
    args = parser.parse_args()

    collection = load_json(args.collection)
    mapping = load_json(args.mapping)

    response_map = build_response_map(mapping, args.responses)
    print(f"Loaded {len(response_map)} endpoint responses")

    injected = inject_responses(collection.get("item", []), response_map)
    print(f"Injected responses into {injected} endpoints")

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w") as f:
        json.dump(collection, f, indent=2)

    print(f"Written: {args.output}")


if __name__ == "__main__":
    main()
