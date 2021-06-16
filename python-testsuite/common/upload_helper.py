from enyx_oe import (
    CollectionStorageAndReadbackEngineManager as CSREManager,
    DataBuffer,
    upload_helper as swig_upload
)

from .buffer_helper import instruction_list_to_buffer

def upload(csre_manager,
           collection_id=0,
           session=0,
           data_mode=CSREManager.CollectionMode_triggerOnce,
           data="",
           instructions=None):
    if instructions is None:
        instructions = []
    data_buffer = DataBuffer(data)
    instr_buffer = instruction_list_to_buffer(instructions)
    return swig_upload(csre_manager, collection_id, session, data_mode, data_buffer.view(), instr_buffer.view())
