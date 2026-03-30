#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

import orjson
from lxml import html
from lxml.etree import _Element


HEADING_TAGS = {"h2", "h3", "h4", "h5", "h6"}
LIST_TAGS = {"ul", "ol"}
CONTENT_BLOCK_TAGS = {
    "p",
    "ul",
    "ol",
    "table",
    "pre",
    "blockquote",
    "aside",
    "div",
    "details",
    "hr",
}
PRISM_CODE_CLASS = "codeBlockContainer"
API_REQUEST_RE = re.compile(r"^(GET|POST|PUT|DELETE|PATCH)\s+(.+)$")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract Binance USD-M Futures mirrored HTML docs into structured JSON files.",
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=Path("docs/api/binance/usds-margined-futures"),
        help="Root directory containing mirrored Binance HTML files.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/binance/usds-margined-futures-json"),
        help="Directory where extracted JSON files will be written.",
    )
    return parser.parse_args()


def clean_text(value: str) -> str:
    cleaned = value.replace("\x00", " ").replace("​", " ").replace("", " ")
    return re.sub(r"\s+", " ", cleaned).strip()


def is_code_like_inline_text(value: str) -> bool:
    stripped = clean_text(value)
    if not stripped:
        return False

    return any(
        token in stripped
        for token in (
            "://",
            "/ws/",
            "/stream?",
            "<streamName",
            "@",
            "{",
            "}",
        )
    ) or stripped.startswith("/")


def inline_text(node: Any) -> str:
    parts: list[str] = []

    def visit(current: Any) -> None:
        if isinstance(current, str):
            parts.append(current)
            return

        text = current.text or ""
        if current.tag == "code":
            if text:
                parts.append(f"`{clean_text(text)}`")
        elif current.tag == "strong" and is_code_like_inline_text(text):
            if text:
                parts.append(f"`{clean_text(text)}`")
        else:
            if text:
                parts.append(text)

            for child in current:
                visit(child)

        tail = current.tail or ""
        if tail:
            parts.append(tail)

    visit(node)
    return clean_text("".join(parts))


def block_text(element: _Element) -> str:
    return inline_text(element)


def code_text(element: _Element) -> str:
    token_lines = element.xpath('.//*[contains(concat(" ", normalize-space(@class), " "), " token-line ")]')
    if token_lines:
        lines = [clean_text("".join(node.itertext())) for node in token_lines]
    else:
        raw = element.xpath("string(.)").replace("\r\n", "\n").replace("\r", "\n")
        lines = [line.rstrip() for line in raw.split("\n")]

    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()

    return "\n".join(lines)


def first_text(element: _Element, xpath: str) -> str | None:
    nodes = element.xpath(xpath)
    if not nodes:
        return None
    if isinstance(nodes[0], str):
        return clean_text(nodes[0]) or None
    return clean_text(" ".join(nodes[0].itertext())) or None


def extract_code_from_element(element: _Element) -> list[dict[str, Any]]:
    code_nodes = element.xpath(".//pre") if element.tag != "pre" else [element]
    blocks: list[dict[str, Any]] = []

    for pre in code_nodes:
        language = None
        classes = pre.get("class", "")
        for class_name in classes.split():
            if class_name.startswith("language-"):
                language = class_name.removeprefix("language-")
                break

        if language is None:
            code = pre.xpath(".//code[1]")
            if code:
                for class_name in code[0].get("class", "").split():
                    if class_name.startswith("language-"):
                        language = class_name.removeprefix("language-")
                        break

        blocks.append(build_code_block("code", language, code_text(pre)))

    return blocks


def build_code_block(block_type: str, language: str | None, text: str) -> dict[str, Any]:
    block: dict[str, Any] = {
        "type": block_type,
        "language": language,
    }

    normalized_language = (language or "").lower()

    if normalized_language in {"json", "javascript"}:
        try:
            parsed = json.loads(text)
            block["language"] = "json"
            block["json"] = parsed
            return block
        except json.JSONDecodeError:
            pass

    block["text"] = text
    return block


def parse_list_item(element: _Element) -> dict[str, Any]:
    nested_blocks = [parse_block(child) for child in element if isinstance(child.tag, str) and child.tag in CONTENT_BLOCK_TAGS]
    normalized_blocks = [block for block in nested_blocks if block is not None]

    text = block_text(element)
    if normalized_blocks and normalized_blocks[0].get("type") == "paragraph":
        text = str(normalized_blocks[0].get("text", "")).strip()
        normalized_blocks = normalized_blocks[1:]

    return {
        "text": text,
        "blocks": normalized_blocks,
    }


def parse_table(element: _Element) -> dict[str, Any]:
    headers = [block_text(node) for node in element.xpath("./thead/tr/th")]
    rows = []
    for row in element.xpath("./tbody/tr | ./tr"):
        cells = [block_text(cell) for cell in row.xpath("./th | ./td")]
        if cells:
            rows.append(cells)

    return {
        "type": "table",
        "headers": headers,
        "rows": rows,
    }


def parse_block(element: _Element) -> dict[str, Any] | None:
    tag = element.tag.lower()

    if tag == "hr":
        return {"type": "separator"}

    if tag == "table":
        return parse_table(element)

    if tag == "blockquote":
        return {
            "type": "blockquote",
            "text": block_text(element),
        }

    if tag == "aside":
        return {
            "type": "notice",
            "text": block_text(element),
        }

    if tag in LIST_TAGS:
        return {
            "type": "list",
            "ordered": tag == "ol",
            "items": [parse_list_item(item) for item in element.xpath("./li")],
        }

    if tag == "pre":
        blocks = extract_code_from_element(element)
        return blocks[0] if blocks else None

    if tag == "div" and PRISM_CODE_CLASS in element.get("class", ""):
        blocks = extract_code_from_element(element)
        return blocks[0] if blocks else None

    if tag == "details":
        summary = first_text(element, "./summary")
        child_blocks = [
            parse_block(child)
            for child in element
            if isinstance(child.tag, str) and child.tag.lower() != "summary" and child.tag.lower() in CONTENT_BLOCK_TAGS
        ]
        return {
            "type": "details",
            "summary": summary,
            "blocks": [block for block in child_blocks if block is not None],
        }

    if tag == "div":
        text = block_text(element)
        if not text:
            return None
        return {
            "type": "html_block",
            "text": text,
        }

    if tag == "p":
        return {
            "type": "paragraph",
            "text": block_text(element),
        }

    text = block_text(element)
    if not text:
        return None

    return {
        "type": tag,
        "text": text,
    }


def select_content_root(document: _Element) -> _Element:
    for xpath in ("//article[1]", "//main[1]", "//body[1]"):
        nodes = document.xpath(xpath)
        if nodes:
            return nodes[0]
    return document


def collect_content_nodes(content_root: _Element) -> tuple[str | None, list[dict[str, Any]], list[dict[str, Any]]]:
    headings = content_root.xpath(".//h1[1]")
    page_title = block_text(headings[0]) if headings else None
    if not headings:
        return page_title, [], []

    h1 = headings[0]
    parent = h1.getparent()
    if parent is None:
        return page_title, [], []

    children = [child for child in parent if isinstance(child.tag, str)]
    try:
        start_index = children.index(h1) + 1
    except ValueError:
        start_index = 0

    intro_blocks: list[dict[str, Any]] = []
    sections: list[dict[str, Any]] = []
    current_section: dict[str, Any] | None = None

    for child in children[start_index:]:
        tag = child.tag.lower()

        if tag in HEADING_TAGS:
            current_section = {
                "title": block_text(child),
                "level": int(tag[1]),
                "id": child.get("id"),
                "blocks": [],
            }
            sections.append(current_section)
            continue

        if tag not in CONTENT_BLOCK_TAGS:
            continue

        block = parse_block(child)
        if block is None:
            continue

        if current_section is None:
            intro_blocks.append(block)
        else:
            current_section["blocks"].append(block)

    return page_title, intro_blocks, sections


def block_values_by_type(blocks: list[dict[str, Any]], block_type: str) -> list[dict[str, Any]]:
    return [block for block in blocks if block.get("type") == block_type]


def section_lookup(sections: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    return {normalize_section_title(section["title"]): section for section in sections}


def normalize_section_title(title: str) -> str:
    normalized = clean_text(title).lower()
    return normalized.rstrip("�").strip()


def build_api_summary(page: dict[str, Any]) -> dict[str, Any]:
    lookup = section_lookup(page["sections"])
    summary: dict[str, Any] = {}

    description_section = (
        lookup.get("api description")
        or lookup.get("stream description")
        or lookup.get("event description")
        or lookup.get("general api information")
    )
    if description_section and description_section["blocks"]:
        summary["description"] = description_section["blocks"][0].get("text")

    http_request = lookup.get("http request")
    if http_request and http_request["blocks"]:
        request_text = http_request["blocks"][0].get("text", "")
        match = API_REQUEST_RE.match(request_text)
        if match:
            summary["http_method"] = match.group(1)
            summary["path"] = match.group(2)
        summary["http_request"] = request_text

    method_section = lookup.get("method")
    if method_section and method_section["blocks"]:
        summary["method"] = method_section["blocks"][0].get("text")

    for key, section_name in (
        ("url_path", "url path"),
        ("stream_name", "stream name"),
        ("event_name", "event name"),
        ("request_weight", "request weight"),
        ("update_speed", "update speed"),
    ):
        section = lookup.get(section_name)
        if section and section["blocks"]:
            summary[key] = section["blocks"][0].get("text")

    request_parameters = lookup.get("request parameters")
    if request_parameters:
        summary["request_parameters_tables"] = block_values_by_type(request_parameters["blocks"], "table")
        summary["request_parameters_text"] = [block.get("text") for block in request_parameters["blocks"] if block.get("text")]

    request_section = lookup.get("request")
    if request_section:
        summary["request_code_blocks"] = block_values_by_type(request_section["blocks"], "code")

    response_section = lookup.get("response example") or lookup.get("response")
    if response_section:
        summary["response_code_blocks"] = block_values_by_type(response_section["blocks"], "code")

    return summary


def classify_page(relative_path: Path) -> dict[str, Any]:
    parts = relative_path.parts
    path_text = "/".join(parts)

    if "/rest-api/" in path_text or path_text.endswith("/rest-api.html"):
        protocol = "rest"
    elif "/websocket-api/" in path_text or path_text.endswith("/websocket-api.html"):
        protocol = "websocket_api"
    elif "/websocket-market-streams/" in path_text:
        protocol = "websocket_stream"
    elif "/user-data-streams/" in path_text:
        protocol = "user_data_stream"
    else:
        protocol = "reference"

    category = parts[-2] if len(parts) >= 2 else "root"
    return {
        "protocol": protocol,
        "category": category,
        "path_parts": list(parts),
    }


def parse_html_file(path: Path, input_root: Path) -> dict[str, Any]:
    document = html.fromstring(path.read_text(encoding="utf-8"))
    content_root = select_content_root(document)
    page_title, intro_blocks, sections = collect_content_nodes(content_root)
    relative_path = path.relative_to(input_root)
    page_type = classify_page(relative_path)

    title = page_title or first_text(document, "//title") or path.stem
    description = first_text(document, "//meta[@name='description']/@content")
    canonical = first_text(document, "//link[@rel='canonical']/@href")

    page = {
        "title": title,
        "meta_description": description,
        "intro_blocks": intro_blocks,
        "sections": sections,
        **page_type,
    }
    page["api_summary"] = build_api_summary(page)
    return page


def write_json(path: Path, payload: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(orjson.dumps(payload, option=orjson.OPT_INDENT_2))


def build_catalog_entry(page: dict[str, Any], output_file: Path, output_root: Path) -> dict[str, Any]:
    summary = page.get("api_summary", {})
    return {
        "title": page["title"],
        "protocol": page["protocol"],
        "category": page["category"],
        "output_file": str(output_file.relative_to(output_root)),
        "http_method": summary.get("http_method"),
        "path": summary.get("path"),
        "method": summary.get("method"),
        "stream_name": summary.get("stream_name"),
        "event_name": summary.get("event_name"),
    }


def extract_docs(input_dir: Path, output_dir: Path) -> None:
    html_files = sorted(path for path in input_dir.rglob("*.html") if path.is_file())
    if not html_files:
        raise SystemExit(f"No HTML files found under {input_dir}")

    catalog: list[dict[str, Any]] = []
    grouped: dict[str, list[dict[str, Any]]] = {
        "rest": [],
        "websocket_api": [],
        "websocket_stream": [],
        "user_data_stream": [],
        "reference": [],
    }

    for html_file in html_files:
        page = parse_html_file(html_file, input_dir)
        output_file = output_dir / html_file.relative_to(input_dir).with_suffix(".json")
        write_json(output_file, page)

        entry = build_catalog_entry(page, output_file, output_dir)
        catalog.append(entry)
        grouped.setdefault(page["protocol"], []).append(entry)

    write_json(
        output_dir / "manifest.json",
        {
            "output_dir": str(output_dir),
            "page_count": len(catalog),
            "pages": catalog,
        },
    )

    for protocol, entries in grouped.items():
        write_json(output_dir / f"{protocol}.json", {"protocol": protocol, "pages": entries})


def main() -> None:
    args = parse_args()
    extract_docs(args.input_dir.resolve(), args.output_dir.resolve())


if __name__ == "__main__":
    main()
