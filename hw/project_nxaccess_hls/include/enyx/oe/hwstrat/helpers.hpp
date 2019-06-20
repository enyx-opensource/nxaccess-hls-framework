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

/// Contains helpers to help trigger, read messages, etc... to be used with Xilinx Vivado HLS.

#pragma once

#include <hls_stream.h>
#include <ap_int.h>

#include "nxoe.hpp"
#include "msg_headers.hpp"

namespace enyx {
namespace oe {
namespace hwstrat {

/// Reads message header from CPU
static void
read_word(struct enyx::oe::hwstrat::cpu2fpga_header& ret_out, ap_uint<64> word_in) {
    ret_out.version = word_in(63,60);
    ret_out.dest = word_in(59,56);
    ret_out.msg_type = word_in(55,52);
    ret_out.ack_request = word_in(51,51);
    ret_out.reserved = word_in(50, 48);
    ret_out.timestamp = word_in(47, 16);
    ret_out.length = word_in(15,0);
}

/// Reads message header from FPGA
static void
read_word(struct enyx::oe::hwstrat::fpga2cpu_header& ret_out, ap_uint<64> word_in) {
    ret_out.version = word_in(63,60);
    ret_out.source = word_in(59,56);
    ret_out.msg_type = word_in(55,52);
    ret_out.error = word_in(51,51);
    ret_out.reserved = word_in(50, 48);
    ret_out.timestamp = word_in(47, 16);
    ret_out.length = word_in(15,0);
}

/// Extract word from cpu2fpga_header struct from CPU
static ap_uint<64>
get_word(const struct enyx::oe::hwstrat::cpu2fpga_header& in) {
    ap_uint<64> out_word;
    out_word(63,60) = in.version;
    out_word(59,56) = in.dest;
    out_word(55,52) = in.msg_type;
    out_word(51,51) = in.ack_request;
    out_word(50, 48) = in.reserved;
    out_word(47, 16) = in.timestamp;
    out_word(15,0) = in.length;
    return out_word;
}


/// Extract word from cpu2fpga_header struct from FPGA
static ap_uint<64>
get_word(const struct enyx::oe::hwstrat::fpga2cpu_header& in) {
    ap_uint<64> out_word;
    out_word(63,60) = in.version;
    out_word(59,56) = in.source;
    out_word(55,52) = in.msg_type;
    out_word(51,51) = in.error;
    out_word(50, 48) = in.reserved;
    out_word(47, 16) = in.timestamp;
    out_word(15,0) = in.length;
    return out_word;
}

}
}
}
