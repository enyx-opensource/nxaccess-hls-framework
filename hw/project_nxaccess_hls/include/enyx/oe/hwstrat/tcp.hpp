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
 * @brief AXI4 bus carrying TCP reply payload from the market.
 */

struct tcp_reply_payload {

    // data bus: payload bytes from ingress TCP segment
    static std::size_t const data_width = 128;
    typedef ap_uint<data_width> Data;
    Data data;

    // data byte qualifier: delimit payload bytes on end-of-packet data word (when last = '1')
    typedef ap_uint<data_width / 8> Keep;
    Keep keep;

    // side-band information: bit 0 set on last word indicates pending packet contains corrupted data
    static std::size_t const user_width = 20;
    typedef ap_uint<user_width> User;
    User user;

    // stream identifier, indicates the TCP session of the received payload
    static std::size_t const id_width = 20;
    typedef ap_uint<id_width> Id;
    Id id;

    // last data word, set at end-of-packet
    typedef ap_uint<1> Last;
    Last last;
};

}}} // Namespaces
