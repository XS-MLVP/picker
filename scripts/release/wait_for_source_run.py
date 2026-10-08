#!/usr/bin/env python3
"""Wait for a validated Tagged CI run to complete successfully before publishing."""

import argparse
import json
import subprocess
import sys
import time


def fetch_run(repository: str, run_id: int) -> dict:
    response = subprocess.check_output(
        ("gh", "api", f"repos/{repository}/actions/runs/{run_id}"),
        text=True,
        timeout=30,
    )
    return json.loads(response)


def wait_for_success(
    repository: str, run_id: int, timeout: int = 3600, interval: int = 10
) -> dict:
    if run_id <= 0 or timeout <= 0 or interval <= 0:
        raise ValueError("Run ID, timeout and polling interval must be positive")
    deadline = time.monotonic() + timeout
    while True:
        run = fetch_run(repository, run_id)
        path = run["path"].split("@", 1)[0]
        if (
            run["repository"]["full_name"] != repository
            or run["id"] != run_id
            or not (
                path == ".github/workflows/release-build.yml"
                or path.endswith("/.github/workflows/release-build.yml")
            )
            or run["event"] not in {"repository_dispatch", "workflow_dispatch"}
        ):
            raise ValueError(f"Run {run_id} is not a dispatched Tagged CI run in {repository}")
        if run["status"] == "completed":
            if run["conclusion"] != "success":
                raise RuntimeError(
                    f"Tagged CI run {run_id} completed with {run['conclusion']}; refusing to publish"
                )
            print(f"Verified successful Tagged CI run: {run_id}", flush=True)
            return run
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(f"Timed out waiting for Tagged CI run {run_id} after {timeout}s")
        print(f"Tagged CI run {run_id} is {run['status']}; waiting for completion", flush=True)
        time.sleep(min(interval, remaining))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("repository")
    parser.add_argument("run_id", type=int)
    parser.add_argument("--timeout", type=int, default=3600)
    args = parser.parse_args()
    try:
        wait_for_success(args.repository, args.run_id, args.timeout)
    except (ValueError, RuntimeError, TimeoutError, subprocess.SubprocessError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
