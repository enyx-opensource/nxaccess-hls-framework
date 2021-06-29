from common.upload_helper import upload


from enyx_oe import (
    CollectionStorageAndReadbackEngineManager as CSREManager,
    makeReplaceWithDataSourceInstruction as makeReplace
)

from .helper_test_case import StandAloneHelperTestCase, str_to_arg

class StandAloneTriggerDisabledCollectionTest(StandAloneHelperTestCase):
    """[summary] Testing triggering a disabled collection and a enabled one
        to check what happens to the argument of the first one.
    """
    def test_disabled_collection_id(self):
        """Triggering a disabled collection and then an enabled.
        """
        self.connect()
        self.assertMalloc(id=0, data_size=256, instr_size=16)
        self.assertMalloc(id=1, data_size=256, instr_size=16)

        data = ['X'*160, ' '*160]
        instrs = [makeReplace(self.hw_fifos[0], 0, 16),
                  makeReplace(self.hw_fifos[1], 32, 16),
                  makeReplace(self.hw_fifos[2], 64, 16),
                  makeReplace(self.hw_fifos[3], 96, 16),
                  makeReplace(self.hw_fifos[4], 128, 16)]

        self.assertFalse(upload(self.csre_manager,
                                collection_id=0,
                                session=0,
                                data_mode=CSREManager.CollectionMode_deactivated,
                                data=data[0],
                                instructions=instrs))

        self.assertFalse(upload(self.csre_manager,
                                collection_id=1,
                                session=0,
                                data_mode=CSREManager.CollectionMode_triggerOnce,
                                data=data[1],
                                instructions=instrs))

        args = [['0'*16, '1'*16, '2'*16, '3'*16, '4'*16],
                ['5'*16, '6'*16, '7'*16, '8'*16, '9'*16]]
        for coll_id in range(2):
            self.assertFalse(self.sa_trigger.trigger(coll_id,
                                                     str_to_arg(args[coll_id][0]),
                                                     str_to_arg(args[coll_id][1]),
                                                     str_to_arg(args[coll_id][2]),
                                                     str_to_arg(args[coll_id][3]),
                                                     str_to_arg(args[coll_id][4])))

        self.reader.wait_for(1, 2)
        self.at_reader.wait_for(1, 2)

        self.assertEqual((len(self.reader.messages), len(self.at_reader.messages)),
                         (1, 1))

        self.expect_updated_message(collection_id=1, session_id=0)
        expected_data = args[0][0] + ' '*16 + \
                        args[0][1] + ' '*16 + \
                        args[0][2] + ' '*16 + \
                        args[0][3] + ' '*16 + \
                        args[0][4] + ' '*16
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)
