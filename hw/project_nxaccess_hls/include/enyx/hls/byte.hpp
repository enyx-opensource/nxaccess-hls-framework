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

#include <ap_int.h>

#include "helpers.hpp"

namespace enyx {

typedef ap_uint<8> byte;

template<typename ByteSequence>
static byte
get_byte(std::size_t offset, ByteSequence const& data)
{
    return get_msb<byte::width>(data << offset * byte::width);
}

template<int BitWidth>
std::size_t
find_first_null_byte(ap_uint<BitWidth> const& data)
{
    assert(BitWidth % 8 == 0);
    static const std::size_t byte_width = BitWidth / 8;

    for (std::size_t i = 0; i != byte_width ; ++i)
        if (get_byte(i, data) == '\0')
            return i;

    return byte_width;
}


template<std::size_t ByteWidth>
struct fill_byte_sequence
{
    ap_uint<ByteWidth * byte::width>
    operator()(byte const fill_byte)
    {
        return (fill_byte << (ByteWidth - 1) * 8) | fill_byte_sequence<ByteWidth - 1>()(fill_byte);
    }
};

template<>
struct fill_byte_sequence<1>
{
    ap_uint<byte::width>
    operator()(byte const fill_byte)
    { return fill_byte; }
};

template<>
struct fill_byte_sequence<0>
{ };

}
