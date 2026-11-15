"""Verify Wiki synchronization cannot remove or overwrite unrelated content."""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest


SPEC = importlib.util.spec_from_file_location(
    'sync_wiki', Path(__file__).resolve().parents[2] / 'scripts/sync_wiki.py'
)
wiki = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(wiki)


class SyncWiki(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.export = self.root / 'export'
        self.checkout = self.root / 'checkout'
        self.export.mkdir()
        self.checkout.mkdir()
        self.enterContext(contextlib.redirect_stdout(io.StringIO()))

    def write_export(self, directory, pages):
        manifest = {}
        for name, text in pages.items():
            (directory / name).write_text(text, encoding='utf-8')
            manifest[name] = hashlib.sha256(text.encode()).hexdigest()
        (directory / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')

    def test_sync_deletes_only_obsolete_generated_pages_and_is_idempotent(self):
        self.write_export(self.checkout, {'Home.md': 'old', 'old-page.md': 'obsolete'})
        (self.checkout / 'Manual.md').write_text('hand maintained')
        self.write_export(self.export, {'Home.md': 'new', 'new-page.md': 'replacement'})
        wiki.sync(self.export, self.checkout)
        snapshot = {p.name: p.read_bytes() for p in self.checkout.iterdir()}
        self.assertNotIn('old-page.md', snapshot)
        self.assertEqual(snapshot['Manual.md'], b'hand maintained')
        self.assertEqual(snapshot['Home.md'], b'new')
        wiki.sync(self.export, self.checkout)
        self.assertEqual(snapshot, {p.name: p.read_bytes() for p in self.checkout.iterdir()})

    def test_initial_sync_replaces_bootstrap_home(self):
        (self.checkout / 'Home.md').write_text('Welcome')
        self.write_export(self.export, {'Home.md': 'Generated'})
        wiki.sync(self.export, self.checkout)
        self.assertEqual((self.checkout / 'Home.md').read_text(), 'Generated')

    def test_bad_checksum_causes_no_partial_update(self):
        self.write_export(self.export, {'Home.md': 'new', 'guide.md': 'guide'})
        (self.export / 'guide.md').write_text('corrupted')
        (self.checkout / 'Home.md').write_text('old')
        with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
            wiki.sync(self.export, self.checkout)
        self.assertEqual((self.checkout / 'Home.md').read_text(), 'old')

    def test_manifest_cannot_escape_checkout(self):
        self.write_export(self.export, {'Home.md': 'new'})
        for name in ('../outside.md', '/outside.md', '.git/config'):
            with self.subTest(name=name):
                (self.checkout / 'manifest.json').write_text(json.dumps({name: '0' * 64}))
                with self.assertRaisesRegex(ValueError, 'Unsafe'):
                    wiki.sync(self.export, self.checkout)

    def test_symlink_cannot_overwrite_external_file(self):
        self.write_export(self.export, {'Home.md': 'new'})
        outside = self.root / 'outside'
        outside.write_text('keep')
        (self.checkout / 'Home.md').symlink_to(outside)
        with self.assertRaisesRegex(ValueError, 'Unsafe Wiki destination'):
            wiki.sync(self.export, self.checkout)
        self.assertEqual(outside.read_text(), 'keep')

    def test_empty_export_cannot_delete_wiki(self):
        self.write_export(self.export, {})
        with self.assertRaisesRegex(ValueError, 'without Home.md'):
            wiki.sync(self.export, self.checkout)

    def test_manual_edits_to_obsolete_page_require_resolution(self):
        self.write_export(self.checkout, {'Home.md': 'old', 'old.md': 'original'})
        (self.checkout / 'old.md').write_text('manual edits')
        self.write_export(self.export, {'Home.md': 'new'})
        with self.assertRaisesRegex(ValueError, 'manual edits'):
            wiki.sync(self.export, self.checkout)
        self.assertEqual((self.checkout / 'Home.md').read_text(), 'old')

    def test_unmanaged_page_conflict_requires_resolution(self):
        self.write_export(self.export, {'Home.md': 'new', 'guide.md': 'generated'})
        (self.checkout / 'guide.md').write_text('manual')
        with self.assertRaisesRegex(ValueError, 'Unmanaged Wiki page conflicts'):
            wiki.sync(self.export, self.checkout)
        self.assertFalse((self.checkout / 'Home.md').exists())


if __name__ == '__main__':
    unittest.main()
