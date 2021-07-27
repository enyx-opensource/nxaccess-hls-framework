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
#include <utility>

#include "../../../enyx/hls/helpers.hpp"
#include "../../../enyx/hls/byte.hpp"
#include "../../../enyx/hfp/hfp.hpp"
#include "../../../enyx/md/hw/nxbus.hpp"

namespace enyx {
namespace oe { 
namespace hwstrat {

void
convert_text_to_trigger(hls::stream<enyx::oe::hwstrat::trigger_command_axi> & output_result, std::string const& content)
{
   
    std::istringstream ss(content);
    enyx::oe::hwstrat::trigger_command read_from_text;
    
    // beware, do not use uint8_t as storage, as C++ stringstream treats this as 'char
    read_from_text.collection_id                  = enyx::get_from_hex_stream_as<uint32_t>(ss);
    read_from_text.valid_arguments                = enyx::get_from_hex_stream_as<uint32_t>(ss);
    read_from_text.arg0(64,127)                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg0(0,63)                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg1(64,127)                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg1(0,63)                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg2(64,127)                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg2(0,63)                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg3(127,64)                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg3(63,0)                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg4(64,127)                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
    read_from_text.arg4(0,63)                     = enyx::get_from_hex_stream_as<uint64_t>(ss);

    output_result.write(read_from_text);
}

std::string get_trigger_command_axi_file_format_header() {

    return std::string("# collection_id 16 | parameters_masks 5 | data0 2x 64 |  data1 2x 64 |  data2 2x 64 |  data3 2x 64 |  data4 2x 64 | \n");
}

std::string get_dma_out_file_header() {

    return std::string("# received packets transmitted on the DMA egress bus\n# IMPORTANT: contents are in hex-string only (no spaces)\n");
}

/// converts a trigger command to text 
std::string 
convert_trigger_command_axi_to_text(hls::stream<enyx::oe::hwstrat::trigger_command_axi>& in)
{
    std::stringstream ret;
    if(!in.empty()) {
        enyx::oe::hwstrat::trigger_command_axi _read_axi = in.read();
        enyx::oe::hwstrat::trigger_command _read = static_cast<enyx::oe::hwstrat::trigger_command>(_read_axi);
        ret << std::hex  << std::noshowbase << std::setw(2) << std::setfill('0') << std::noshowbase
            << std::setw(4)   <<  std::noshowbase  <<  static_cast<uint16_t>(_read.collection_id) << " "
            <<  std::setw(2)  << std::setfill('0') <<  static_cast<uint16_t>(_read.valid_arguments) <<  " "

            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg0 >> 64) << " "
            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg0) << " "

            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg1 >> 64) << " "
            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg1)<< " "

            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg2 >> 64) << " "
            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg2 & 0xffffffffffffffff)<< " "

            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg3 >> 64) << " "
            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg3 & 0xffffffffffffffff)<< " "

            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg4 >> 64) << " "
            <<  std::setw(16) << std::setfill('0') <<  static_cast<uint64_t>(_read.arg4 & 0xffffffffffffffff);
    }
    return ret.str();
} // convert_trigger_command_axi_to_text

/// converts a DMA data packet into text and end-of-line boolean
std::pair<std::string, bool> 
convert_dma_out_to_text(hls::stream<enyx::hfp::dma_user_channel_data_out>& in)
{
    std::stringstream ret;
    bool end_of_line;
    if(!in.empty()) {
        auto _read_axi = in.read();
        ret 
            << std::setfill('0')
            << std::setw(16) << std::hex << static_cast<uint64_t>(_read_axi.data >> 64)
            << std::setw(16) << std::hex << static_cast<uint64_t>(_read_axi.data & 0xffffffffffffffff);
        end_of_line = _read_axi.last == 1;
    }
    return std::make_pair(ret.str(), end_of_line);
}

}}} // Namespaces
