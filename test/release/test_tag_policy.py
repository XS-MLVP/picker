import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[2] / "scripts/release/tag_policy.py"
spec = importlib.util.spec_from_file_location("tag_policy", SCRIPT)
tag_policy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tag_policy)


class TagPolicyTests(unittest.TestCase):
    def test_merged_feature_commit_is_not_on_master_first_parent_line(self):
        with tempfile.TemporaryDirectory(prefix="picker-tag-policy-") as directory:
            def git(*args):
                return subprocess.check_output(
                    ("git", "-C", directory, *args), text=True
                ).strip()

            git("init", "-q", "-b", "master")
            git("config", "user.name", "Tag Policy Test")
            git("config", "user.email", "test@example.invalid")
            git("commit", "-q", "--allow-empty", "-m", "base")
            git("checkout", "-q", "-b", "feature")
            git("commit", "-q", "--allow-empty", "-m", "feature")
            feature = git("rev-parse", "HEAD")
            git("tag", "v9.0.0")
            git("checkout", "-q", "master")
            git("merge", "-q", "--no-ff", "feature", "-m", "merge feature")
            merge = git("rev-parse", "HEAD")
            git("tag", "v0.9.1")
            git("update-ref", "refs/remotes/origin/master", merge)

            with patch.object(tag_policy, "git", side_effect=git), patch.object(
                tag_policy, "merged_master_pr", side_effect=lambda sha: sha == merge
            ):
                master_line = tag_policy.first_parent_commits()
                self.assertNotIn(feature, master_line)
                self.assertIn(merge, master_line)
                self.assertEqual(tag_policy.latest_master_tag(master_line), "v0.9.1")

    def test_latest_ignores_off_master_and_non_merge_tags(self):
        tags = {
            ("tag", "--list"): "v9.0.0\nv0.9.3\nv0.9.2\nv0.9.1\nold-date-tag",
            ("rev-parse", "refs/tags/v9.0.0^{commit}"): "feature",
            ("rev-parse", "refs/tags/v0.9.3^{commit}"): "direct",
            ("rev-parse", "refs/tags/v0.9.2^{commit}"): "merge2",
        }
        with patch.object(tag_policy, "git", side_effect=lambda *args: tags[args]), patch.object(
            tag_policy, "merged_master_pr", side_effect=lambda sha: sha == "merge2"
        ):
            self.assertEqual(
                tag_policy.latest_master_tag({"direct", "merge2"}), "v0.9.2"
            )

    def test_off_master_commit_does_not_query_github(self):
        with patch.object(tag_policy, "merged_master_pr") as query:
            self.assertFalse(tag_policy.eligible("feature", {"merge"}))
            query.assert_not_called()

    def test_merge_result_requires_matching_master_pr(self):
        prs = [
            {"merged_at": "2026-01-01", "base": {"ref": "other"}, "merge_commit_sha": "sha"},
            {"merged_at": "2026-01-01", "base": {"ref": "master"}, "merge_commit_sha": "different"},
        ]
        with patch.dict("os.environ", {"GITHUB_REPOSITORY": "owner/repo"}), patch.object(
            tag_policy.subprocess, "check_output", return_value=json.dumps(prs)
        ):
            self.assertFalse(tag_policy.merged_master_pr("sha"))
            prs.append(
                {"merged_at": "2026-01-01", "base": {"ref": "master"}, "merge_commit_sha": "sha"}
            )
            with patch.object(
                tag_policy.subprocess, "check_output", return_value=json.dumps(prs)
            ):
                self.assertTrue(tag_policy.merged_master_pr("sha"))


if __name__ == "__main__":
    unittest.main()
