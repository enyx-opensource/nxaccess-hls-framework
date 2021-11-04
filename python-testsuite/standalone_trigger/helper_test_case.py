
from common import first_stack_server_ip
from common.test_helper import HelperTestCase
from common.tcp_session_helper import MuteServerHelper

from common.read_helper import (
    get_metadata,
    get_msg_type,
    get_tcp_stack_message_status
)


from enyx_oe import (
    Accelerator,
    CollectionMetadata,
    IPv4EndPoint,
    TCPStatus_established,
    TCPStatus_opening,
    retrieve_sandbox_info
)

from enyx_oe_hwstrat_hls_demo import StandAloneTrigger

def str_to_arg(value: str):
    return list(map(ord, value))

class StandAloneHelperTestCase(HelperTestCase):
    def setUp(self):
        super().setUp()
        self.sa_trigger = StandAloneTrigger(0)
        if not self.is_hls_demo():
            self.tearDown()
            self.skipTest("This firmware does not contains the HLS demo")

        self.server = MuteServerHelper(first_stack_server_ip, 0)
        self.addCleanup(self.server.stop)
        self.server_endpoint = IPv4EndPoint(first_stack_server_ip, self.server.server.server_address[1])
        print("Mute TCP server opened on {}".format(self.server_endpoint))
        self.assertGreaterEqual(len(self.hw_fifos), 5)

    def tearDown(self):
        self.sa_trigger = None
        return super().tearDown()

    def is_hls_demo(self):
        accelerator = Accelerator(0)
        info = retrieve_sandbox_info(accelerator)
        if not info:
            return False
        return info.applicationName == "enyx_oe_hwstrat_hls_demo"

    def connect(self):
        """Connect to the tcp server
        """
        self.assertEqual(len(self.reader.messages), 0)
        open_ret = self.session_manager.openSession(0, self.server_endpoint)
        print("Session 0 opened to {}".format(self.server_endpoint))
        self.assertFalse(open_ret)
        self.reader.wait_for(2, 1)
        self.assertEqual(len(self.reader.messages), 2)
        self.assertEqual(get_msg_type(self.reader.messages[0]), CollectionMetadata.MessageType_TCPStackStatus)
        self.assertEqual(get_tcp_stack_message_status(self.reader.messages[0]), TCPStatus_opening)
        self.assertEqual(get_msg_type(self.reader.messages[1]), CollectionMetadata.MessageType_TCPStackStatus)
        self.assertEqual(get_tcp_stack_message_status(self.reader.messages[1]), TCPStatus_established)
        self.reader.messages.pop(0)
        self.reader.messages.pop(0)
        self.assertEqual(len(self.reader.messages), 0)
        self.addCleanup(self.session_manager.closeSession, 0)

    def assertMalloc(self, id, data_size, instr_size):
        malloc_ret = self.csre_manager.malloc(id, data_size, instr_size)
        self.assertFalse(malloc_ret.first)

    def expect_updated_message(self, collection_id: int, session_id: int):
        self.assertGreaterEqual(len(self.reader.messages), 1)
        msg = self.reader.messages.pop(0)
        metadata = get_metadata(msg)
        print("Metadata of next message: {}".format(metadata))
        self.assertEqual(metadata['type'], CollectionMetadata.MessageType_UpdatedMessage)
        self.assertEqual(metadata['collection_id'], collection_id)
        self.assertEqual(metadata['session_id'], session_id)

    def expected_audit_trail_sent(self, session_id: int, data: bytes):
        self.assertGreaterEqual(len(self.at_reader.messages), 1)
        msg = self.at_reader.messages.pop(0)
        print("Next audit trail message: {}".format(msg))
        self.assertEqual(msg['type'], CollectionMetadata.MessageType_UpdatedMessage)
        self.assertEqual(msg['session_id'], session_id)
        self.assertEqual(msg['data'], data)