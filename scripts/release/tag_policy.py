#!/usr/bin/env python3
"""Validate release tags against merged PR results on master's first-parent line."""

import argparse
import json
import os
import re
import subprocess
import sys


RELEASE_TAG = re.compile(r"v(\d+)\.(\d+)\.(\d+)")


def git(*args: str) -> str:
    return subprocess.check_output(("git", *args), text=True).strip()


def first_parent_commits() -> set[str]:
    return set(git("rev-list", "--first-parent", "origin/master").splitlines())


def merged_master_pr(sha: str) -> bool:
    repository = os.environ["GITHUB_REPOSITORY"]
    response = subprocess.check_output(
        ("gh", "api", f"repos/{repository}/commits/{sha}/pulls"), text=True
    )
    pull_requests = json.loads(response)
    return any(
        pr.get("merged_at")
        and pr.get("base", {}).get("ref") == "master"
        and pr.get("merge_commit_sha") == sha
        for pr in pull_requests
    )


def eligible(sha: str, master_line: set[str]) -> bool:
    return sha in master_line and merged_master_pr(sha)


def latest_master_tag(master_line: set[str]) -> str | None:
    candidates = []
    for tag in git("tag", "--list").splitlines():
        match = RELEASE_TAG.fullmatch(tag)
        if match:
            candidates.append((tuple(map(int, match.groups())), tag))
    for _, tag in sorted(candidates, reverse=True):
        sha = git("rev-parse", f"refs/tags/{tag}^{{commit}}")
        if eligible(sha, master_line):
            return tag
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("verify", "latest"))
    parser.add_argument("commit", nargs="?", default="HEAD")
    args = parser.parse_args()
    master_line = first_parent_commits()
    if args.command == "verify":
        sha = git("rev-parse", f"{args.commit}^{{commit}}")
        if not eligible(sha, master_line):
            print(
                f"{sha} is not the result of a merged master PR on master's first-parent line",
                file=sys.stderr,
            )
            return 1
        print(f"Verified master merge result: {sha}")
    else:
        tag = latest_master_tag(master_line)
        if tag:
            print(tag)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
