from common.test_helper import retry_with_timeout
from common.upload_helper import upload


from enyx_oe import (
    makeReplaceWithDataSourceInstruction as makeReplace
)

from .helper_test_case import StandAloneHelperTestCase, str_to_arg

class StandAloneTriggerDifferentCollectionIdTest(StandAloneHelperTestCase):
    """[summary] Testing different value of collection id in stand alone trigger.
    """
    def test_different_collection_id(self):
        """Test trigger with different collection id
        """
        self.connect()

        collection_ids = range(8192)

        for coll_id in collection_ids:
            self.assertMalloc(id=coll_id, data_size=16, instr_size=1)

        data = 'X'*16
        instrs = [makeReplace(self.hw_fifos[0], 0, 16)]

        for coll_id in collection_ids:
            self.assertFalse(upload(self.csre_manager,
                                    collection_id=coll_id,
                                    session=0,
                                    data=data,
                                    instructions=instrs))

        for coll_id in collection_ids:
            def trigger():
                self.sa_trigger.trigger(coll_id, str_to_arg(str(coll_id).rjust(16)))
            self.assertFalse(retry_with_timeout(cmd=trigger, timeout=1))

        self.reader.wait_for(len(collection_ids), 10)
        self.at_reader.wait_for(len(collection_ids), 10)

        self.assertEqual((len(self.reader.messages), len(self.at_reader.messages)),
                         (len(collection_ids), len(collection_ids)))

        for coll_id in collection_ids:
            self.expect_updated_message(collection_id=coll_id, session_id=0)
            expected_data = str(coll_id).rjust(16)
            self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)
