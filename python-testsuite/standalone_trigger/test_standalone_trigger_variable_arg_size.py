from random import randint

from common.test_helper import generate_random_str
from common.upload_helper import upload

from enyx_oe import (
    makeReplaceWithDataSourceInstruction as makeReplace
)

from .helper_test_case import StandAloneHelperTestCase, str_to_arg

class StandAloneTriggerVariableArgumentSizeTest(StandAloneHelperTestCase):
    """Test with different size of arguments for the standalone trigger
    """
    def test_all_args_different_size(self):
        """Test only using all arguments with different sizes.
        """
        self.connect()

        data = ' '*256
        instrs = [makeReplace(self.hw_fifos[0], 0, 1),
                  makeReplace(self.hw_fifos[1], 32, 2),
                  makeReplace(self.hw_fifos[2], 64, 3),
                  makeReplace(self.hw_fifos[3], 96, 4),
                  makeReplace(self.hw_fifos[4], 128, 5),]
        self.assertMalloc(id=0, data_size=256, instr_size=16)
        self.assertFalse(upload(self.csre_manager,
                                collection_id=0,
                                session=0,
                                data=data,
                                instructions=instrs))

        arg0 = "0123456789abcdef"
        arg1 = "ghijklmnopqrstuv"
        arg2 = "wxyzABCDEFGHIJKL"
        arg3 = "MNOPQRSTUVWXYZ,."
        arg4 = "<>!?;/:%*+=-_#]["
        self.assertFalse(self.sa_trigger.trigger(0,
                                                 str_to_arg(arg0[:1]),
                                                 str_to_arg(arg1[:2]),
                                                 str_to_arg(arg2[:3]),
                                                 str_to_arg(arg3[:4]),
                                                 str_to_arg(arg4[:5])))

        expected_data = arg0[:1] + ' ' * (16 + 15) + \
                        arg1[:2] + ' ' * (16 + 14) + \
                        arg2[:3] + ' ' * (16 + 13) + \
                        arg3[:4] + ' ' * (16 + 12) + \
                        arg4[:5] + ' ' * (16 + 11 + 96)

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)
        self.assertEqual(len(self.reader.messages), 1)
        self.assertEqual(len(self.at_reader.messages), 1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)

    def test_all_args_different_size_random(self):
        """Test only using all arguments with different sizes.
        """
        self.connect()
        nb_of_test_vector = 128

        test_vector = [[{"data": generate_random_str(16), "size": randint(1, 16)}
                        for _ in range(5)]
                       for i in range(nb_of_test_vector)]

        for coll_id, vector in enumerate(test_vector):
            self.assertMalloc(id=coll_id, data_size=128, instr_size=16)

        for coll_id, vector in enumerate(test_vector):
            data = ' '*80
            instrs = [makeReplace(self.hw_fifos[0], 0, vector[0]["size"]),
                      makeReplace(self.hw_fifos[1], 16, vector[1]["size"]),
                      makeReplace(self.hw_fifos[2], 32, vector[2]["size"]),
                      makeReplace(self.hw_fifos[3], 48, vector[3]["size"]),
                      makeReplace(self.hw_fifos[4], 64, vector[4]["size"]),]
            self.assertFalse(upload(self.csre_manager,
                                    collection_id=coll_id,
                                    session=0,
                                    data=data,
                                    instructions=instrs))

            self.assertFalse(self.sa_trigger.trigger(coll_id,
                                                     str_to_arg(vector[0]["data"]),
                                                     str_to_arg(vector[1]["data"]),
                                                     str_to_arg(vector[2]["data"]),
                                                     str_to_arg(vector[3]["data"]),
                                                     str_to_arg(vector[4]["data"])))

        self.reader.wait_for(len(test_vector), 10)
        self.at_reader.wait_for(len(test_vector), 10)
        self.assertEqual(len(self.reader.messages), len(test_vector))
        self.assertEqual(len(self.at_reader.messages), len(test_vector))

        for coll_id, vector in enumerate(test_vector):
            self.expect_updated_message(collection_id=coll_id, session_id=0)
            expected_data = vector[0]["data"][:vector[0]["size"]] + ' '*(16 - vector[0]["size"]) + \
                            vector[1]["data"][:vector[1]["size"]] + ' '*(16 - vector[1]["size"]) + \
                            vector[2]["data"][:vector[2]["size"]] + ' '*(16 - vector[2]["size"]) + \
                            vector[3]["data"][:vector[3]["size"]] + ' '*(16 - vector[3]["size"]) + \
                            vector[4]["data"][:vector[4]["size"]] + ' '*(16 - vector[4]["size"])
            self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)
