#!/usr/bin/env python3
"""Check and fix Markdown link consistency across the repository.

Subcommands
-----------
list-files       List all git-tracked .md files.
list-links       List every Markdown link in every tracked .md file.
list-links-in    List links in a specific file.
check            Verify all links are root-relative and targets exist.
fix              Attempt automatic fixes (make paths root-relative,
                 resolve name-only matches when the current path is wrong).

All paths printed and expected in links are relative to the project root
(the directory that contains .git).
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path, PurePosixPath
from dataclasses import dataclass, field

# Match [text](target) but not images ![alt](src)
LINK_RE = re.compile(r"(?<!!)\[([^\]]*)\]\(([^)]+)\)")


# ── helpers ──────────────────────────────────────────────────────────────────

def project_root() -> Path:
    """Walk upward until we find .git."""
    p = Path(__file__).resolve().parent
    while p != p.parent:
        if (p / ".git").exists():
            return p
        p = p.parent
    sys.exit("error: could not find project root (.git)")


def git_tracked_md(root: Path) -> list[Path]:
    """Return sorted list of git-tracked *.md files (relative to *root*)."""
    out = subprocess.check_output(
        ["git", "ls-files", "--cached", "*.md", "**/*.md"],
        cwd=root,
        text=True,
    )
    return sorted({root / l for l in out.strip().splitlines() if l})


@dataclass
class Link:
    file: Path          # absolute path of the .md file containing the link
    line: int           # 1-based line number
    text: str           # display text inside [ ]
    target: str         # raw href inside ( )
    col: int = 0        # 0-based column of the opening [


@dataclass
class Problem:
    link: Link
    kind: str           # "not-root-relative", "missing-target", "wrong-dir"
    detail: str = ""
    suggestion: str = ""


def extract_links(filepath: Path) -> list[Link]:
    """Parse all Markdown links from a file."""
    links: list[Link] = []
    try:
        text = filepath.read_text(errors="replace")
    except OSError:
        return links
    for lineno, line in enumerate(text.splitlines(), 1):
        for m in LINK_RE.finditer(line):
            raw_target = m.group(2)
            # skip external URLs, anchors, mailto, and C++ lambda signatures
            if raw_target.startswith(("http://", "https://", "#", "mailto:")):
                continue
            if m.group(1) == "" and ("," in raw_target or "::" in raw_target):
                continue
            links.append(Link(
                file=filepath,
                line=lineno,
                text=m.group(1),
                target=raw_target,
                col=m.start(),
            ))
    return links


def build_name_index(root: Path, md_files: list[Path]) -> dict[str, list[Path]]:
    """Map filename/dirname -> list of absolute paths where that name appears.

    Indexes all git-tracked files and their parent directories.
    """
    idx: dict[str, list[Path]] = {}
    dirs: set[Path] = set()
    for md in md_files:
        idx.setdefault(md.name, []).append(md)
    # Also index non-md tracked files so source-code links resolve.
    out = subprocess.check_output(
        ["git", "ls-files", "--cached"],
        cwd=root,
        text=True,
    )
    for line in out.strip().splitlines():
        p = root / line
        idx.setdefault(p.name, []).append(p)
        # collect all parent directories up to root
        d = p.parent
        while d != root and d not in dirs:
            dirs.add(d)
            d = d.parent
    # index directories by their name
    for d in dirs:
        idx.setdefault(d.name, []).append(d)
    # deduplicate
    for k in idx:
        idx[k] = sorted(set(idx[k]))
    return idx


def resolve_target(link: Link, root: Path) -> Path:
    """Resolve a link target to an absolute path."""
    raw = link.target.split("#")[0].split("?")[0]  # strip fragment/query
    if not raw:
        return link.file  # self-link via anchor
    target_posix = PurePosixPath(raw)
    if raw.startswith("/"):
        return root / str(target_posix).lstrip("/")
    # relative to the file's directory
    return (link.file.parent / Path(raw)).resolve()


def to_root_relative(abspath: Path, root: Path) -> str:
    """Convert absolute path to /root-relative string."""
    try:
        rel = abspath.relative_to(root)
    except ValueError:
        return str(abspath)
    return "/" + str(PurePosixPath(rel))


def path_exists(target_path: Path, raw: str) -> bool:
    """Check if target_path exists, handling :line suffixes."""
    if target_path.exists():
        return True
    if ":" in target_path.name:
        base = target_path.parent / target_path.name.rsplit(":", 1)[0]
        return base.exists()
    return False


def strip_path(raw: str) -> str:
    """Strip fragment, query, and :line suffix for filesystem checks."""
    raw = raw.split("#")[0].split("?")[0]
    return raw


def find_by_name(raw: str, name_idx: dict[str, list[Path]], root: Path) -> tuple[list[Path], str]:
    """Look up candidates by filename. Returns (candidates, bare_name)."""
    name = PurePosixPath(strip_path(raw)).name
    if ":" in name:
        name = name.rsplit(":", 1)[0]
    return name_idx.get(name, []), name


def build_suggestion(candidate: Path, link: Link, root: Path) -> str:
    """Build a full suggestion string preserving :line and #fragment."""
    base = to_root_relative(candidate, root)
    raw_no_fragment = link.target.split("#")[0]
    suffix = ""
    if ":" in raw_no_fragment:
        suffix = ":" + raw_no_fragment.rsplit(":", 1)[1]
    fragment = ""
    if "#" in link.target:
        fragment = "#" + link.target.split("#", 1)[1]
    return base + suffix + fragment


def check_link(link: Link, root: Path, name_idx: dict[str, list[Path]]) -> Problem | None:
    raw = strip_path(link.target)
    if not raw:
        return None

    is_relative = not raw.startswith("/")

    # Step 1: resolve to an absolute path regardless of style
    if is_relative:
        resolved = resolve_target(link, root)
        global_form = to_root_relative(resolved, root)
    else:
        resolved = root / raw.lstrip("/")
        global_form = raw

    # Step 2: for relative links, convert to global and check
    if is_relative:
        # 2a: try interpreting as already root-relative (missing leading /)
        as_root = "/" + raw
        as_root_path = root / raw
        if path_exists(as_root_path, as_root):
            return Problem(
                link=link,
                kind="not-root-relative",
                detail=f"relative path, exists as {as_root}",
                suggestion=as_root,
            )

        # 2b: resolve relative to the file's directory
        target_path = root / global_form.lstrip("/")
        if path_exists(target_path, global_form):
            return Problem(
                link=link,
                kind="not-root-relative",
                detail=f"relative path resolves to {global_form}",
                suggestion=global_form,
            )

        # 2c: neither works - try name lookup
        candidates, name = find_by_name(raw, name_idx, root)
        if len(candidates) == 1:
            return Problem(
                link=link,
                kind="not-root-relative",
                detail=f"relative path, unique name match found",
                suggestion=build_suggestion(candidates[0], link, root),
            )
        if len(candidates) > 1:
            locs = ", ".join(to_root_relative(c, root) for c in candidates[:5])
            return Problem(
                link=link,
                kind="not-root-relative",
                detail=f"relative path, ambiguous name matches: {locs}",
                suggestion="",
            )
        return Problem(
            link=link,
            kind="not-root-relative",
            detail="relative path, target does not exist and no name match found",
            suggestion="",
        )

    # Step 3: already root-relative - check existence
    if path_exists(resolved, raw):
        return None

    # missing: try name-based lookup
    candidates, name = find_by_name(raw, name_idx, root)
    if len(candidates) == 1:
        return Problem(
            link=link,
            kind="wrong-dir",
            detail=f"target missing; unique name match at {to_root_relative(candidates[0], root)}",
            suggestion=build_suggestion(candidates[0], link, root),
        )
    if len(candidates) > 1:
        locs = ", ".join(to_root_relative(c, root) for c in candidates[:5])
        return Problem(
            link=link,
            kind="missing-target",
            detail=f"target missing; ambiguous name matches: {locs}",
        )
    return Problem(
        link=link,
        kind="missing-target",
        detail="target does not exist and no name match found",
    )


def apply_fix(problem: Problem, root: Path) -> bool:
    """Rewrite the link in-place. Returns True if changed."""
    if not problem.suggestion:
        return False
    fp = problem.link.file
    lines = fp.read_text(errors="replace").splitlines(keepends=True)
    idx = problem.link.line - 1
    if idx >= len(lines):
        return False
    old_line = lines[idx]

    old_target = problem.link.target
    new_target = problem.suggestion
    # preserve fragment
    if "#" in old_target and "#" not in new_target:
        new_target += "#" + old_target.split("#", 1)[1]

    new_line = old_line.replace(f"]({old_target})", f"]({new_target})", 1)
    if new_line == old_line:
        return False
    lines[idx] = new_line
    fp.write_text("".join(lines))
    return True


# ── subcommands ──────────────────────────────────────────────────────────────

def cmd_list_files(args: argparse.Namespace) -> int:
    root = project_root()
    for f in git_tracked_md(root):
        print(f.relative_to(root))
    return 0


def cmd_list_links(args: argparse.Namespace) -> int:
    root = project_root()
    md_files = git_tracked_md(root)
    for f in md_files:
        for link in extract_links(f):
            rel = f.relative_to(root)
            print(f"{rel}:{link.line}  [{link.text}]({link.target})")
    return 0


def cmd_list_links_in(args: argparse.Namespace) -> int:
    root = project_root()
    target = (root / args.file).resolve()
    if not target.exists():
        sys.exit(f"error: {args.file} does not exist")
    for link in extract_links(target):
        print(f"{link.line}:{link.col}  [{link.text}]({link.target})")
    return 0


def cmd_check(args: argparse.Namespace) -> int:
    root = project_root()
    md_files = git_tracked_md(root)
    name_idx = build_name_index(root, md_files)

    all_links: list[Link] = []
    for f in md_files:
        all_links.extend(extract_links(f))

    problems: list[Problem] = []
    for link in all_links:
        p = check_link(link, root, name_idx)
        if p:
            problems.append(p)

    if not problems:
        print(f"OK: {len(all_links)} links checked, no problems found.")
        return 0

    by_kind: dict[str, list[Problem]] = {}
    for p in problems:
        by_kind.setdefault(p.kind, []).append(p)

    for kind in ("not-root-relative", "wrong-dir", "missing-target"):
        group = by_kind.get(kind, [])
        if not group:
            continue
        print(f"\n{'=' * 60}")
        print(f"  {kind}  ({len(group)} issues)")
        print(f"{'=' * 60}")
        for p in group:
            rel = p.link.file.relative_to(root)
            print(f"  {rel}:{p.link.line}")
            print(f"    link:   [{p.link.text}]({p.link.target})")
            print(f"    issue:  {p.detail}")
            if p.suggestion:
                print(f"    fix ->  {p.suggestion}")
            print()

    total = len(problems)
    fixable = sum(1 for p in problems if p.suggestion)
    print(f"Summary: {total} problems ({fixable} auto-fixable), {len(all_links)} links checked.")
    return 1


def cmd_fix(args: argparse.Namespace) -> int:
    root = project_root()
    md_files = git_tracked_md(root)
    name_idx = build_name_index(root, md_files)

    all_links: list[Link] = []
    for f in md_files:
        all_links.extend(extract_links(f))

    problems: list[Problem] = []
    for link in all_links:
        p = check_link(link, root, name_idx)
        if p:
            problems.append(p)

    if not problems:
        print(f"OK: {len(all_links)} links checked, nothing to fix.")
        return 0

    fixed = 0
    skipped = 0
    for p in problems:
        if not p.suggestion:
            skipped += 1
            continue
        rel = p.link.file.relative_to(root)
        if args.dry_run:
            print(f"  would fix {rel}:{p.link.line}  {p.link.target} -> {p.suggestion}")
            fixed += 1
        else:
            if apply_fix(p, root):
                print(f"  fixed {rel}:{p.link.line}  {p.link.target} -> {p.suggestion}")
                fixed += 1
            else:
                print(f"  FAILED {rel}:{p.link.line}  could not apply fix")
                skipped += 1

    action = "would fix" if args.dry_run else "fixed"
    print(f"\n{action} {fixed}, skipped {skipped} (no suggestion), "
          f"{len(all_links)} links total.")
    return 0 if skipped == 0 else 1


# ── main ─────────────────────────────────────────────────────────────────────

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Check and fix Markdown link consistency.",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("list-files", help="List all git-tracked .md files.")
    sub.add_parser("list-links", help="List every link in all .md files.")

    p_in = sub.add_parser("list-links-in", help="List links in a specific file.")
    p_in.add_argument("file", help="Path to .md file (relative to project root).")

    sub.add_parser("check", help="Verify all links are root-relative and exist.")

    p_fix = sub.add_parser("fix", help="Auto-fix broken links.")
    p_fix.add_argument("-n", "--dry-run", action="store_true",
                       help="Show what would change without writing.")

    args = parser.parse_args()
    return {
        "list-files": cmd_list_files,
        "list-links": cmd_list_links,
        "list-links-in": cmd_list_links_in,
        "check": cmd_check,
        "fix": cmd_fix,
    }[args.command](args)


if __name__ == "__main__":
    raise SystemExit(main())
