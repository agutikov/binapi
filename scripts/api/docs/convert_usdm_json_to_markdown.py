#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert extracted Binance USD-M Futures JSON docs into Markdown files.",
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=Path(".local/binance/usds-margined-futures-json"),
        help="Directory containing extracted JSON files.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/binance/usds-margined-futures-md"),
        help="Directory where Markdown files will be written.",
    )
    return parser.parse_args()


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def markdown_escape(text: str) -> str:
    return text.replace("|", "\\|")


def format_inline_code(text: str) -> str:
    return text


def fenced_json(value: Any) -> str:
    return "```json\n" + json.dumps(value, indent=2, ensure_ascii=False) + "\n```\n"


def render_table(block: dict[str, Any]) -> list[str]:
    headers = [format_inline_code(str(value)) for value in block.get("headers", [])]
    rows = [[format_inline_code(str(cell)) for cell in row] for row in block.get("rows", [])]

    if not headers and not rows:
        return []

    if not headers and rows:
        headers = [f"col_{index + 1}" for index in range(max(len(row) for row in rows))]

    lines = ["| " + " | ".join(markdown_escape(header) for header in headers) + " |"]
    lines.append("| " + " | ".join("---" for _ in headers) + " |")

    for row in rows:
        padded = row + [""] * (len(headers) - len(row))
        lines.append("| " + " | ".join(markdown_escape(cell) for cell in padded[: len(headers)]) + " |")

    lines.append("")
    return lines


def render_list(block: dict[str, Any], indent: int = 0) -> list[str]:
    prefix = "  " * indent
    lines: list[str] = []

    for index, item in enumerate(block.get("items", []), start=1):
        marker = f"{index}." if block.get("ordered") else "-"
        text = format_inline_code(str(item.get("text", "")).strip())
        lines.append(f"{prefix}{marker} {text}".rstrip())
        for child in item.get("blocks", []):
            lines.extend(render_block(child, indent + 1))

    lines.append("")
    return lines


def render_block(block: dict[str, Any], indent: int = 0) -> list[str]:
    block_type = block.get("type")

    if block_type == "paragraph":
        return [format_inline_code(str(block.get("text", ""))), ""]

    if block_type == "blockquote":
        text = format_inline_code(str(block.get("text", "")))
        return [*(f"> {line}" for line in text.splitlines() or [""]), ""]

    if block_type == "notice":
        text = format_inline_code(str(block.get("text", "")))
        return [f"> **Note**: {text}", ""]

    if block_type == "list":
        return render_list(block, indent)

    if block_type == "table":
        return render_table(block)

    if block_type == "code":
        language = str(block.get("language") or "")
        if "json" in block:
            return [fenced_json(block["json"])]
        text = str(block.get("text", ""))
        return [f"```{language}".rstrip(), text, "```", ""]

    if block_type == "details":
        lines = [f"**{format_inline_code(str(block.get('summary', 'Details')))}**", ""]
        for child in block.get("blocks", []):
            lines.extend(render_block(child, indent))
        return lines

    if block_type == "separator":
        return ["---", ""]

    text = format_inline_code(str(block.get("text", "")))
    return [text, ""] if text else []


def render_api_summary(page: dict[str, Any]) -> list[str]:
    summary = page.get("api_summary", {})
    if not summary:
        return []

    lines = ["## API Summary", ""]
    ordered_keys = [
        "description",
        "http_method",
        "path",
        "http_request",
        "method",
        "url_path",
        "stream_name",
        "event_name",
        "request_weight",
        "update_speed",
    ]

    for key in ordered_keys:
        if key in summary:
            lines.append(f"- **{key}**: {format_inline_code(str(summary[key]))}")

    lines.append("")

    if summary.get("request_parameters_tables"):
        lines.append("### Request Parameters")
        lines.append("")
        for table in summary["request_parameters_tables"]:
            lines.extend(render_table(table))

    if summary.get("request_parameters_text"):
        lines.append("### Request Parameter Notes")
        lines.append("")
        for item in summary["request_parameters_text"]:
            lines.append(f"- {format_inline_code(str(item))}")
        lines.append("")

    if summary.get("request_code_blocks"):
        lines.append("### Request Examples")
        lines.append("")
        for block in summary["request_code_blocks"]:
            lines.extend(render_block(block))

    if summary.get("response_code_blocks"):
        lines.append("### Response Examples")
        lines.append("")
        for block in summary["response_code_blocks"]:
            lines.extend(render_block(block))

    return lines


def render_page(page: dict[str, Any]) -> str:
    lines = [f"# {page.get('title', 'Untitled')}", ""]

    lines.append(f"- **protocol**: {page.get('protocol', '')}")
    lines.append(f"- **category**: {page.get('category', '')}")
    if page.get("meta_description"):
        lines.append(f"- **meta_description**: {format_inline_code(str(page['meta_description']))}")
    lines.append("")

    lines.extend(render_api_summary(page))

    if page.get("intro_blocks"):
        lines.append("## Introduction")
        lines.append("")
        for block in page["intro_blocks"]:
            lines.extend(render_block(block))

    for section in page.get("sections", []):
        level = int(section.get("level", 2))
        heading = "#" * min(max(level, 2), 6)
        lines.append(f"{heading} {section.get('title', '')}")
        lines.append("")
        for block in section.get("blocks", []):
            lines.extend(render_block(block))

    return "\n".join(lines).rstrip() + "\n"


def convert_file(input_path: Path, input_dir: Path, output_dir: Path) -> None:
    page = load_json(input_path)
    if not isinstance(page, dict) or "title" not in page:
        return

    relative_path = input_path.relative_to(input_dir)
    output_path = output_dir / relative_path.with_suffix(".md")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(render_page(page), encoding="utf-8")


def main() -> None:
    args = parse_args()
    input_dir = args.input_dir.resolve()
    output_dir = args.output_dir.resolve()

    for path in sorted(input_dir.rglob("*.json")):
        if path.name in {
            "manifest.json",
            "rest.json",
            "websocket_api.json",
            "websocket_stream.json",
            "user_data_stream.json",
            "reference.json",
        }:
            continue
        convert_file(path, input_dir, output_dir)


if __name__ == "__main__":
    main()
