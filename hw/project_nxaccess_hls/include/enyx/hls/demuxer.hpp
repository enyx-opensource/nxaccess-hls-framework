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

#include <stdint.h>
#include <hls_stream.h>

namespace enyx {
namespace hls_tools {

/// Demux inuput stream to N children streams.
template<typename Id,
         std::size_t BusCount,
         typename Word>

class demuxer {
 public:
    typedef Word data_in_word;
    typedef Word data_out_word;
    static const std::size_t bus_count = BusCount;

    static void
    p_demux(hls::stream<data_in_word> &in,
          hls::stream<data_out_word> (&out)[bus_count]) {
        #pragma HLS INLINE recursive
        #pragma HLS PIPELINE enable_flush
        if(!in.empty()) {
            Word const _read = in.read();
            for(int i = 0; i != bus_count; ++i) {
                out[i].write(_read);
            }
        }

    }
};
}} // Namespaces