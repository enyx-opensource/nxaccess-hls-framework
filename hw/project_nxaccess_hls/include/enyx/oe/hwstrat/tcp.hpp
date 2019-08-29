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

namespace enyx {
namespace oe {
namespace hwstrat {

/**
 * @brief AXI4 bus that contains the TCP payloads coming from the market.
 */
struct tcp_reply_payload {
    static std::size_t const data_width = 128;
    ap_uint<data_width> data;    // 'data', the TCP payloads.

    static std::size_t const user_width = 8;
    ap_uint<user_width> user;    // 'user' contains the TCP Session ID.

//    static std::size_t const id_width = 1;
//    ap_uint<id_width> id;

//    static std::size_t const destr_width = 1;
//    ap_uint<id_width> dest;

    ap_uint<1> last;
};

// non-working code below

// We inherit the ap_axiu AXI bus, and add a metadata traits

//typedef ap_axiu<128, 8, 1, 1> tcp_replies;

////struct tcp_replies: public ap_axiu<128, 8, 1, 1> {

////    // AXI:data, user,id,dest.
////    // 'user' contains the TCP Session ID.
////    // 'data', the TCP payloads.
////    static std::size_t const data_width = 128;
////    static std::size_t const user_width = 8;
////    static std::size_t const id_width = 1;
////    static std::size_t const dest_width = 1;
////};


}}} // Namespaces
