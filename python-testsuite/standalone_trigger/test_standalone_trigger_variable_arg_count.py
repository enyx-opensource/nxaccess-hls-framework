from common.upload_helper import upload

from enyx_oe import (
    makeReplaceWithDataSourceInstruction as makeReplace
)

from .helper_test_case import StandAloneHelperTestCase, str_to_arg

class StandAloneTriggerVariableArgumentCountTest(StandAloneHelperTestCase):
    def test_first_arg(self):
        """Test only using the first argument
        """
        self.connect()

        data = ' '*64
        instrs = [makeReplace(self.hw_fifos[0], 0, 16)]
        self.assertMalloc(id=0, data_size=256, instr_size=16)
        self.assertFalse(upload(self.csre_manager,
                                collection_id=0,
                                session=0,
                                data=data,
                                instructions=instrs))

        arg0 = "0123456789abcdef"
        self.assertFalse(self.sa_trigger.trigger(0, str_to_arg(arg0)))
        expected_data = arg0  + ' '* (64 - len(arg0))

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)

    def test_two_args(self):
        """Test only using the first two arguments
        """
        self.connect()

        data = ' '*64
        instrs = [makeReplace(self.hw_fifos[0], 0, 16),
                  makeReplace(self.hw_fifos[1], 32, 16)]
        self.assertMalloc(id=0, data_size=256, instr_size=16)
        self.assertFalse(upload(self.csre_manager,
                                collection_id=0,
                                session=0,
                                data=data,
                                instructions=instrs))

        arg0 = "0123456789abcdef"
        arg1 = "ghijklmnopqrstuv"
        self.assertFalse(self.sa_trigger.trigger(0,
                                                 str_to_arg(arg0),
                                                 str_to_arg(arg1)))

        expected_data = arg0  + ' '*16 + arg1 + ' '*16

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)


    def test_three_args(self):
        """Test only using the first three arguments
        """
        self.connect()

        data = ' '*96
        instrs = [makeReplace(self.hw_fifos[0], 0, 16),
                  makeReplace(self.hw_fifos[1], 32, 16),
                  makeReplace(self.hw_fifos[2], 64, 16)]
        self.assertMalloc(id=0, data_size=256, instr_size=16)
        self.assertFalse(upload(self.csre_manager,
                                collection_id=0,
                                session=0,
                                data=data,
                                instructions=instrs))

        arg0 = "0123456789abcdef"
        arg1 = "ghijklmnopqrstuv"
        arg2 = "wxyzABCDEFGHIJKL"
        self.assertFalse(self.sa_trigger.trigger(0,
                                                 str_to_arg(arg0),
                                                 str_to_arg(arg1),
                                                 str_to_arg(arg2)))

        expected_data = arg0  + ' '*16 + arg1 + ' '*16 + arg2 + ' '*16

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)

    def test_four_args(self):
        """Test only using the first four arguments
        """
        self.connect()

        data = ' '*128
        instrs = [makeReplace(self.hw_fifos[0], 0, 16),
                  makeReplace(self.hw_fifos[1], 32, 16),
                  makeReplace(self.hw_fifos[2], 64, 16),
                  makeReplace(self.hw_fifos[3], 96, 16)]
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
        self.assertFalse(self.sa_trigger.trigger(0,
                                                 str_to_arg(arg0),
                                                 str_to_arg(arg1),
                                                 str_to_arg(arg2),
                                                 str_to_arg(arg3)))

        expected_data = arg0 + ' '*16 + \
                        arg1 + ' '*16 + \
                        arg2 + ' '*16 + \
                        arg3 + ' '*16

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)

    def test_all_args(self):
        """Test only using all arguments.
        """
        self.connect()

        data = ' '*256
        instrs = [makeReplace(self.hw_fifos[0], 0, 16),
                  makeReplace(self.hw_fifos[1], 32, 16),
                  makeReplace(self.hw_fifos[2], 64, 16),
                  makeReplace(self.hw_fifos[3], 96, 16),
                  makeReplace(self.hw_fifos[4], 128, 16),]
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
                                                 str_to_arg(arg0),
                                                 str_to_arg(arg1),
                                                 str_to_arg(arg2),
                                                 str_to_arg(arg3),
                                                 str_to_arg(arg4)))

        expected_data = arg0 + ' ' * 16 + \
                        arg1 + ' ' * 16 + \
                        arg2 + ' ' * 16 + \
                        arg3 + ' ' * 16 + \
                        arg4 + ' ' * (16 + 96)

        self.reader.wait_for(1)
        self.at_reader.wait_for(1)
        self.assertEqual(len(self.reader.messages), 1)
        self.assertEqual(len(self.at_reader.messages), 1)

        self.expect_updated_message(collection_id=0, session_id=0)
        self.expected_audit_trail_sent(session_id=0, data=expected_data.encode('utf-8'))

        self.assertEqual(len(self.reader.messages), 0)
        self.assertEqual(len(self.at_reader.messages), 0)
