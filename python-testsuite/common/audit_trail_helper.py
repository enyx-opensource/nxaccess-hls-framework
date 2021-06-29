from threading import Thread
from time import sleep
from common.read_helper import get_metadata
from common.wait import waitFor
from enyx_oe import AuditTrailManager
from enyx_oe import AuditTrailAsyncReader as swig_AuditTrailAsyncReader
from enyx_oe import CollectionMetadata, DataBuffer, MessageSent, MessageReceived

class AuditTrailAsyncReader(Thread):
    def __init__(self, board):
        self.manager = AuditTrailManager(board)
        self.reader = swig_AuditTrailAsyncReader(self.manager)
        self.messages = []
        self.should_stop = False
        Thread.__init__(self)
        self.start()

    def wait_for(self, number_of_messages, timeout=1):
        def check_nb_message():
            return len(self.messages) >= number_of_messages
        return waitFor(check_nb_message, timeout)

    def stop(self):
        self.should_stop = True
        self.join()

    def run(self):
        self.reader.start()
        while not self.should_stop:
            msg = bytes(self.reader.pop())
            if len(msg) > 0:
                data = DataBuffer(msg)
                metadata = get_metadata(msg)
                to_add = {'type': metadata['type'], 'session_id': metadata['session_id']}
                if metadata['type'] == CollectionMetadata.MessageType_UpdatedMessage:
                    audit_msg = MessageSent(data.view()).message()
                    to_add['data'] = bytes(DataBuffer(audit_msg).value())
                elif metadata['type'] == CollectionMetadata.MessageType_TCPReplyPayload:
                    audit_msg = MessageReceived(data.view()).message()
                    to_add['data'] = bytes(DataBuffer(audit_msg).value())
                else:
                    print("Unexpected message type received on Audit Trail %i" % metadata['type'])
                    continue
                self.messages.append(to_add)
            else:
                sleep(0.05)
        self.reader.stop()
