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

    ap_uint<1> last;
};



}}} // Namespaces
