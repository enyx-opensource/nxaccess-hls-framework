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

#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>
#include <iomanip>
#include <ap_int.h>

#include "../../../enyx/hls/helpers.hpp"
#include "../../../enyx/hls/byte.hpp"
#include "../../../enyx/md/hw/nxbus.hpp"

namespace enyx {
namespace md { 
namespace hw {
            
void
convert_nxbus_string_to_nxbus_axi(hls::stream<enyx::md::hw::nxbus_axi> & result, std::string const& content)
{
    // We want to read this kind of line: 
    // # EOE|id|code|order_id   |buy|qty    | price          | timestamp| instr_ascii                  | instr_bin|instr_id|data0         | data1  | data2
    // 01 00 95 0000000000000000 00 00000030 13A9875F0CBF7981 00000000 00000000000000000000000000000000 00000000 00000002 0102030405060708 00000000 0000000000000000

    std::istringstream ss(content);
    enyx::md::hw::nxbus nxbus_word;
    
    // Beware, do not use uint8_t as storage, as C++ stringstream treats this as 'char
    nxbus_word.end_of_extra              = enyx::get_from_hex_stream_as<uint32_t>(ss);
    nxbus_word.market_internal_id        = enyx::get_from_hex_stream_as<uint32_t>(ss);
    nxbus_word.opcode                    = enyx::get_from_hex_stream_as<uint32_t>(ss);
    nxbus_word.order_id                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.buy_nsell                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.qty                       = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.price                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.timestamp                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.instr_ascii               = enyx::get_from_hex_stream_as<uint64_t>(ss); // FIXME most probably wrong as doesn't fit on 64bits
    nxbus_word.instr_bin                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.instr_id                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.data0                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.data1                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    nxbus_word.data2                     = enyx::get_from_hex_stream_as<uint64_t>(ss);

    result.write(static_cast<enyx::md::hw::nxbus_axi>(nxbus_word));
}
}}} // Namespaces