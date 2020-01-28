import os
import unittest
from pathlib import Path

from helpers import hls_project_repository


class HlsProjectRepositoryTestCase(unittest.TestCase):
    def test_path(self):
        hls = hls_project_repository.HlsProjectRepository()
        self.assertIsInstance(hls.path, Path)
        pwd = os.getcwd()
        self.assertTrue(str(hls.path) in pwd)

    def test_list_hdl_files(self):
        hls = hls_project_repository.HlsProjectRepository()
        for language in hls.SUPPORTED_HDL_LANGUAGES:
            files = hls.list_hdl_files(language=language)
            self.assertIsInstance(files, list)

    def test_get_last_commit(self):
        hls = hls_project_repository.HlsProjectRepository()
        commit = hls.get_last_commit()
        self.assertIsInstance(commit, str)
        self.assertTrue(commit)


if __name__ == '__main__':
    unittest.main()
