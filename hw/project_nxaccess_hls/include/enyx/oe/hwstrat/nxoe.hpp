//--------------------------------------------------------------------------------
//--! Licensed Materials - Property of ENYX
//--!
//--! Organization:          Enyx
//--! Project Identifier:    010 - Enyx nxAccess HLS Framework
//--! Author:                Raphael Charolois (raphael.charolois@enyx.com)
//--!
//--! © Copyright            Enyx 2019
//--! © Copyright Notice:    The source code for this program is not published or otherwise divested of its trade secrets, 
//--!                        irrespective of what has been deposited with the U.S. Copyright Office.
//--------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <ap_int.h>
#include <hls_stream.h>
#include <ap_fixed.h>

namespace enyx {
namespace oe {
namespace hwstrat {

// Order Entry Constants
struct trigger_command_axi; //fwd decl

// Meta prog for command bus definition
struct trigger_meta_size {
    // nxBus Sizes
    static std::size_t const TRIGGER_SIZE_COLLECTION_ID        = 16; // 16 bits
    static std::size_t const TRIGGER_SIZE_ARGUMENTS       = 8; // 8 bits: three MSB bits reserved, remaining bits for each of the five arguments
    static std::size_t const TRIGGER_SIZE_VALID_ARGUMENTS       = 5; // 8 bits: three MSB bits reserved, remaining bits for each of the five arguments
    static std::size_t const _TRIGGER_SIZE_DATA_ARGUMENT       = 128; // 128 bits

    // we need the size for each argument
    static std::size_t const TRIGGER_SIZE_ARG0       = _TRIGGER_SIZE_DATA_ARGUMENT; // 128 bits
    static std::size_t const TRIGGER_SIZE_ARG1       = _TRIGGER_SIZE_DATA_ARGUMENT; // 128 bits
    static std::size_t const TRIGGER_SIZE_ARG2       = _TRIGGER_SIZE_DATA_ARGUMENT; // 128 bits
    static std::size_t const TRIGGER_SIZE_ARG3       = _TRIGGER_SIZE_DATA_ARGUMENT; // 128 bits
    static std::size_t const TRIGGER_SIZE_ARG4       = _TRIGGER_SIZE_DATA_ARGUMENT; // 128 bits

};

// this meta struct is only here to avoid coding errors
// C++ allows offsetof() to get the offset of the BYTE-aligned sturct, representing bit-aligned FPGA struct
#pragma pack(1) // ensure C++ packs structures correclty
struct trigger_meta_struct{
    char TRIGGER_SIZE_COLLECTION_ID[trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID];
    char TRIGGER_SIZE_ARGUMENTS[trigger_meta_size::TRIGGER_SIZE_ARGUMENTS];
    char TRIGGER_SIZE_ARG0[trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT];
    char TRIGGER_SIZE_ARG1[trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT];
    char TRIGGER_SIZE_ARG2[trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT];
    char TRIGGER_SIZE_ARG3[trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT];
    char TRIGGER_SIZE_ARG4[trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT];
};

/// AXI bus for triggers
struct trigger_command_axi {

    static std::size_t const data_width = trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID + \
                                          trigger_meta_size::TRIGGER_SIZE_ARGUMENTS + \
                                          trigger_meta_size::TRIGGER_SIZE_VALID_ARGUMENTS * trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT;
    ap_uint<data_width> data;

    ap_uint<1> last;
};


#define ARG_TO_MEMBER(__member, __struct_meta_member) __member = \
    arg.data(offsetof(trigger_meta_struct, __struct_meta_member), \
             offsetof(trigger_meta_struct, __struct_meta_member) + trigger_meta_size::__struct_meta_member -1)

#define MEMBER_TO_ARG(__member, __struct_meta_member) \
    arg.data(offsetof(trigger_meta_struct, __struct_meta_member), \
             offsetof(trigger_meta_struct, __struct_meta_member) + trigger_meta_size::__struct_meta_member -1) = __member

/// Command bus for triggers
struct trigger_command {

    trigger_command() {} // default

    /// Conversion operator from AXI bus
    trigger_command(trigger_command_axi const _arg) {
    	trigger_command_axi arg;
    	arg = _arg;
    	arg.data.reverse(); // reverse bit ordering (end_of_extra is MSB on AXI)
        ARG_TO_MEMBER(collection_id,     TRIGGER_SIZE_COLLECTION_ID) ;
        ARG_TO_MEMBER(valid_arguments, TRIGGER_SIZE_ARGUMENTS) ;
        ARG_TO_MEMBER(arg0,             TRIGGER_SIZE_ARG0) ;
        ARG_TO_MEMBER(arg1,             TRIGGER_SIZE_ARG1) ;
        ARG_TO_MEMBER(arg2,             TRIGGER_SIZE_ARG2) ;
        ARG_TO_MEMBER(arg3,             TRIGGER_SIZE_ARG3) ;
        ARG_TO_MEMBER(arg4,             TRIGGER_SIZE_ARG4) ;
    }

    /// Conversion operator to AXI bus
    operator trigger_command_axi() {
        trigger_command_axi arg;
        MEMBER_TO_ARG(collection_id,     TRIGGER_SIZE_COLLECTION_ID) ;
        MEMBER_TO_ARG(valid_arguments, TRIGGER_SIZE_ARGUMENTS) ;
        MEMBER_TO_ARG(arg0,             TRIGGER_SIZE_ARG0) ;
        MEMBER_TO_ARG(arg1,             TRIGGER_SIZE_ARG1) ;
        MEMBER_TO_ARG(arg2,             TRIGGER_SIZE_ARG2) ;
        MEMBER_TO_ARG(arg3,             TRIGGER_SIZE_ARG3) ;
        MEMBER_TO_ARG(arg4,             TRIGGER_SIZE_ARG4) ;
        arg.data.reverse();
        return arg;
    }

    ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID>  collection_id;
    ap_uint<trigger_meta_size::TRIGGER_SIZE_ARGUMENTS> valid_arguments  ; // valid arguments in the following bus
    ap_uint<trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT> arg0;
    ap_uint<trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT> arg1;
    ap_uint<trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT> arg2;
    ap_uint<trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT> arg3;
    ap_uint<trigger_meta_size::_TRIGGER_SIZE_DATA_ARGUMENT> arg4;
};

#undef ARG_TO_MEMBER
#undef MEMBER_TO_ARG

static std::size_t const TCP_REPLIES_DATA_WIDTH       = 128;

// Master function, can be used for advanced users.
static void
    trigger_collection_internal(hls::stream<trigger_command_axi> & trigger_axibus_out,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARGUMENTS> valid_arguments,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG1> arg1,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG2> arg2,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG3> arg3,
                                ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG4> arg4)
    {
        trigger_command output_trigger;
        trigger_command_axi output_trigger_axi;

        output_trigger.collection_id = collection_id;
        output_trigger.valid_arguments = valid_arguments;
        output_trigger.arg0 = arg0;
        output_trigger.arg1 = arg1;
        output_trigger.arg2 = arg2;
        output_trigger.arg3 = arg3;
        output_trigger.arg4 = arg4;
        trigger_command_axi output = static_cast<trigger_command_axi>(output_trigger);
        output.last = 1;
        trigger_axibus_out.write(output);
    }

// Trigger without any arguments
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b00000,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0);
    }

// Trigger with 1 argument
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b00001,
                                    arg0,
                                    0,
                                    0,
                                    0,
                                    0);
    }

// Trigger with 2 arguments
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG1> arg1)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b00011,
                                    arg0,
                                    arg1,
                                    0,
                                    0,
                                    0);
    }

// Trigger with 3 arguments
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG1> arg1,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG2> arg2)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b00111,
                                    arg0,
                                    arg1,
                                    arg2,
                                    0,
                                    0);
    }

// Trigger with 4 arguments
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG1> arg1,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG2> arg2,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG3> arg3)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b01111,
                                    arg0,
                                    arg1,
                                    arg2,
                                    arg3,
                                    0);
    }

// Trigger with 5 arguments
static void
    trigger_collection(hls::stream<trigger_command_axi> & trigger_axibus_out,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> collection_id,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG0> arg0,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG1> arg1,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG2> arg2,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG3> arg3,
                       ap_uint<trigger_meta_size::TRIGGER_SIZE_ARG4> arg4)
    {
        trigger_collection_internal(trigger_axibus_out,
                                    collection_id,
                                    0b11111,
                                    arg0,
                                    arg1,
                                    arg2,
                                    arg3,
                                    arg4);
    }


}}} // Namespaces
