from threading import Thread
from time import sleep
from enum import IntEnum

from common.wait import waitFor
from enyx_oe import DataBuffer
from enyx_oe import read as swig_read
from enyx_oe import AsyncReader as swig_AsyncReader
from enyx_oe import CollectionMetadata, BaseMessage, FailedMessage
from enyx_oe import TCPReplyPayload, TCPStatusMessage

def read(oue_manager):
    return bytes(swig_read(oue_manager))

class MessageType(IntEnum):
    RawMessage = 0,
    UpdatedMessage = 1,
    TCPReplyPayload = 2,
    TCPStackStatus = 3,
    KillSwitchEvent = 4,
    FailedMessage = 5,
    DataSourceReportingMessage = 6

class TcpStatus(IntEnum):
    # closed = 0,
    # listen =1 ,
    # syn_rcvd =2,
    # syn_sent =3,
    # established =4,
    # close_wait =5,
    # last_ack =6,
    # fin_wait_1 =7,
    # fin_wait_2 =8,
    # closing =9 ,
    # time_wait = 10
    closed = 0,
    opening = 1,
    established = 2,
    closing = 3

def get_metadata(data_read):
    assert(isinstance(data_read, bytes))
    data = DataBuffer(data_read)
    metadata = BaseMessage(data.view()).metadata()
    ret = {}
    ret['type'] = metadata.messageType()
    ret['Type'] = MessageType(metadata.messageType())
    ret['collection_id'] = metadata.collectionId()
    ret['session_id'] = metadata.tcpSessionId()
    ret['length'] = metadata.length()
    ret['timestamp'] = metadata.timestamp()
    ret['status'] = metadata.status() # this is vector of uint8_t
    return ret


def get_msg_type(data_read):
    return get_metadata(data_read)['type']

def get_msg_length(data_read):
    return get_metadata(data_read)['length']

def get_failed_message_reason(data_read):
    assert(isinstance(data_read, bytes))
    assert(get_msg_type(data_read) == CollectionMetadata.MessageType_FailedMessage)
    data = DataBuffer(data_read)
    ret = {}
    ret['tcp'] = FailedMessage(data.view()).tpcSessionNotOpened()
    ret['fifo'] = FailedMessage(data.view()).listOfFieldsBelowThreshold()
    return ret

def get_tcp_stack_message_status(data_read):
    assert(isinstance(data_read, bytes))
    assert(get_msg_type(data_read) == CollectionMetadata.MessageType_TCPStackStatus)
    data = DataBuffer(data_read)
    return TCPStatusMessage(data.view()).status()

def get_tcp_reply_message(data_read):
    assert(isinstance(data_read, bytes))
    assert(get_msg_type(data_read) == CollectionMetadata.MessageType_TCPReplyPayload)
    data = DataBuffer(data_read)
    msg = TCPReplyPayload(data.view()).message()
    return bytes(DataBuffer(msg).value())

class AsyncReader(Thread):
    def __init__(self, oue_manager):
        self.oue_manager = oue_manager
        self.reader = swig_AsyncReader(oue_manager)
        self.messages = list()
        self.should_stop = False
        Thread.__init__(self)
        self.start()

    def wait_for(self, number_of_message, timeout=1):
        def check_nb_message():
            return len(self.messages) >= number_of_message
        return waitFor(check_nb_message, timeout)

    def stop(self):
        self.should_stop = True
        self.join()

    def run(self):
        self.reader.start()
        while not self.should_stop:
            vector_msg = self.reader.pop()
            msg = bytes(vector_msg)
            if len(msg) > 0:
                self.messages.append(msg)
            else:
                sleep(0.05)
        self.reader.stop()
