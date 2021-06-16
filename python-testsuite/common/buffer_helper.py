from enyx_oe import InstructionBuffer

def instruction_list_to_buffer(instructions):
    instr_buffer_data = []
    for instruction in instructions:
        instr_buffer_data.append(instruction.raw())
    return InstructionBuffer(instr_buffer_data)