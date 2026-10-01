import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[2] / "scripts/release/wait_for_source_run.py"
spec = importlib.util.spec_from_file_location("wait_for_source_run", SCRIPT)
source_run = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source_run)


def run(status="completed", conclusion="success", **overrides):
    return {
        "id": 123,
        "repository": {"full_name": "owner/repo"},
        "path": ".github/workflows/release-build.yml",
        "event": "repository_dispatch",
        "status": status,
        "conclusion": conclusion,
        **overrides,
    }


class SourceRunTests(unittest.TestCase):
    def test_waits_through_pending_states_until_success(self):
        snapshots = [run("queued", None), run("in_progress", None), run()]
        with patch.object(source_run, "fetch_run", side_effect=snapshots), patch.object(
            source_run.time, "sleep"
        ) as sleep, patch.object(source_run.time, "monotonic", return_value=0):
            self.assertEqual(source_run.wait_for_success("owner/repo", 123), snapshots[-1])
            self.assertEqual(sleep.call_count, 2)

    def test_completed_success_does_not_wait(self):
        with patch.object(source_run, "fetch_run", return_value=run()), patch.object(
            source_run.time, "sleep"
        ) as sleep:
            source_run.wait_for_success("owner/repo", 123)
            sleep.assert_not_called()

    def test_failed_or_cancelled_source_never_passes(self):
        for conclusion in ("failure", "cancelled", "timed_out", "skipped", "neutral", None):
            with self.subTest(conclusion=conclusion), patch.object(
                source_run, "fetch_run", return_value=run(conclusion=conclusion)
            ), patch.object(source_run.time, "sleep") as sleep:
                with self.assertRaisesRegex(RuntimeError, "refusing to publish"):
                    source_run.wait_for_success("owner/repo", 123)
                sleep.assert_not_called()

    def test_wrong_source_is_rejected_before_waiting(self):
        for overrides in (
            {"id": 456},
            {"repository": {"full_name": "other/repo"}},
            {"path": ".github/workflows/ci.yml"},
            {"event": "pull_request"},
        ):
            with self.subTest(overrides=overrides), patch.object(
                source_run, "fetch_run", return_value=run("in_progress", None, **overrides)
            ), patch.object(source_run.time, "sleep") as sleep:
                with self.assertRaisesRegex(ValueError, "not a dispatched Tagged CI"):
                    source_run.wait_for_success("owner/repo", 123)
                sleep.assert_not_called()

    def test_manual_source_and_qualified_workflow_path_are_accepted(self):
        snapshot = run(
            event="workflow_dispatch",
            path="owner/repo/.github/workflows/release-build.yml@refs/heads/master",
        )
        with patch.object(source_run, "fetch_run", return_value=snapshot):
            self.assertEqual(source_run.wait_for_success("owner/repo", 123), snapshot)

    def test_wait_is_bounded_and_last_sleep_is_clipped_to_deadline(self):
        with patch.object(source_run, "fetch_run", return_value=run("in_progress", None)), patch.object(
            source_run.time, "monotonic", side_effect=[0, 3595, 3600]
        ), patch.object(source_run.time, "sleep") as sleep:
            with self.assertRaisesRegex(TimeoutError, "after 3600s"):
                source_run.wait_for_success("owner/repo", 123)
            sleep.assert_called_once_with(5)

    def test_invalid_inputs_do_not_query_github(self):
        for values in ({"run_id": 0}, {"run_id": 123, "timeout": 0}, {"run_id": 123, "interval": 0}):
            with self.subTest(values=values), patch.object(source_run, "fetch_run") as fetch:
                with self.assertRaises(ValueError):
                    source_run.wait_for_success("owner/repo", **values)
                fetch.assert_not_called()


if __name__ == "__main__":
    unittest.main()
