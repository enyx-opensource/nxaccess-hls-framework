"""
Test command line arguments
"""

import unittest

from flows import firmware


class ArgumentsTestCase(unittest.TestCase):

    def test_help(self):
        """
        Test that usage is shown when no arguments are passed

        :return: nothing
        """

        help_arguments = ['-h', '--help']
        for arg in help_arguments:
            with self.assertRaises(SystemExit) as context:
                argv = [arg]
                firmware.rebuild(argv)
            self.assertEqual(0, context.exception.code)


if __name__ == '__main__':
    unittest.main()
