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

#include <hls_stream.h>

#include "word_traits.hpp"

namespace enyx {
namespace hls_tools {

template<typename Id,
         std::size_t BusCount,
         typename Word>

class arbiter
{
public:
    typedef Word data_in_word;
    typedef Word data_out_word;
    static const std::size_t bus_count = BusCount;

public:
    static void
    p_arbitrate(hls::stream<data_in_word> (&in)[bus_count],
                hls::stream<data_out_word> & out)
    {
        #pragma HLS INLINE recursive
        #pragma HLS PIPELINE enable_flush
        static std::size_t last_used_bus_id;
        #pragma HLS RESET variable=last_used_bus_id
        static enum { IDLE, FORWARDING } state;
        #pragma HLS RESET variable=state

        switch (state)
        {
        case IDLE:
            for (std::size_t i = 1; i <= bus_count; ++i)
            {
                std::size_t bus_id = last_used_bus_id + i;
                if (bus_id >= bus_count)
                    bus_id -= bus_count;

                if (! in[bus_id].empty())
                {
                    last_used_bus_id = bus_id;
                    data_in_word const word_in = in[bus_id].read();
                    out.write(word_in);
                    if (! is_last(word_in))
                        state = FORWARDING;
                    break;
                }
            }
            break;

        case FORWARDING:
            if (! in[last_used_bus_id].empty())
            {
                data_in_word const word_in = in[last_used_bus_id].read();
                out.write(word_in);
                if (is_last(word_in))
                    state = IDLE;
            }
            break;
        }
    }
};
}} // Namespaces