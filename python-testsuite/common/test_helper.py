import unittest
import os
import logging
from random import choice
from string import digits, ascii_letters
from time import time
from common.tcp_session_helper import reset_all_sessions
from common.tcp_ip_stack_helper import setup_first_tcp_ip_stack
from common.audit_trail_helper import AuditTrailAsyncReader
from common.wait import waitFor as realWaitFor
from common.read_helper import AsyncReader
from enyx_oe import (
    Accelerator,
    CollectionStorageAndReadbackEngineManager,
    FifoManager,
    KillSwitchManager as KSManager,
    OrderUpdateEngineManager,
    TCPSessionManager
)
import enyx_oe

# FIXME/ Retro Compat
wait_for = realWaitFor


class HelperTestCase(unittest.TestCase):
    """Base class for all testing that instantiate useful module
    """
    def setUp(self):
        logging.basicConfig(level=logging.INFO)

        self.board = Accelerator(0)

        if os.getenv('ENYX_FORCE_BOARD_RESET'):
            self.board.reset()
        # Setup Network stuff first
        self.assertFalse(setup_first_tcp_ip_stack(self.board),
                         "Unable to configure first Ip Stack")
        self.session_manager = TCPSessionManager(self.board)
        reset_all_sessions(self.session_manager)
        self.addCleanup(reset_all_sessions, self.session_manager)


        self.csre_manager = None
        self.oue_manager = None
        self.reader = None
        self.at_reader = None

        self.csre_manager = CollectionStorageAndReadbackEngineManager(self.board)
        self.oue_manager = OrderUpdateEngineManager(self.board)

        self.reader = AsyncReader(self.oue_manager)
        self.assertEqual(len(self.reader.messages), 0, "Communication channel not empty before sending")
        self.addCleanup(self.reader.stop)

        self.at_reader = AuditTrailAsyncReader(self.board)
        self.addCleanup(self.at_reader.stop)
        self.assertEqual(len(self.at_reader.messages), 0, "Communication channel not empty before sending")

        self.hw_fifos = FifoManager(self.oue_manager).getDedicatedFifosIDs()

    def tearDown(self):
        self.at_reader = None
        self.reader = None
        self.oue_manager = None
        self.csre_manager = None

    def _disable_usunsed_features(self):
        KSManager(self.board).disable()
        fifo_manager = FifoManager(self.oue_manager)
        fifo_manager.disableAll()
        fifo_manager.clearAll()
        fifo_manager.manager().setDataSourceReportingRate(0)

def generate_random_str(size, str_choice=digits + ascii_letters):
    return ''.join(choice(str_choice) for _ in range(size))

def retry_with_timeout(cmd, timeout=1):
    ret = cmd()
    timeout = time() + timeout
    while time() < timeout:
        if not ret:
            break
        ret = cmd()
    return ret
