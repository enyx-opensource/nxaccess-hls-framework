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

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_fixed.h>

namespace enyx {

template<int MsbSize, typename Type>
inline ap_uint<MsbSize>
as_msb(Type const& in)
{
    assert(MsbSize >= std::size_t(Type::width));

    return in, ap_uint<MsbSize - Type::width>(0);
}

template<int MsbSize>
inline ap_uint<MsbSize>
as_msb(ap_uint<MsbSize> const& in)
{
    return in;
}

template<int SizeOut, typename Type>
inline ap_uint<SizeOut>
as_lsb(Type const& in)
{
    assert(SizeOut >= std::size_t(Type::width));

    return ap_uint<SizeOut - Type::width>(0), in;
}

template<std::size_t MsbSize, typename Type>
inline ap_uint<MsbSize>
get_msb(Type const& a)
{
    assert(std::size_t(Type::width) >= MsbSize);
    return a(Type::width - 1, Type::width - MsbSize);
}

template<std::size_t LsbSize, typename Type>
inline ap_uint<LsbSize>
get_lsb(Type const& a)
{
    assert(std::size_t(Type::width) >= LsbSize);
    return a(LsbSize - 1, 0);
}

template<typename Type>
inline Type
zero_msb(std::size_t size, Type const& a)
{
    assert(size <= std::size_t(Type::width));

    ap_uint<Type::width> const mask = ap_uint<Type::width>(-1) >> size;
    return a & mask;
}

template<typename Type>
inline Type
zero_lsb(std::size_t size, Type const& a)
{
    assert(size <= std::size_t(Type::width));

    ap_uint<Type::width> const mask = ap_uint<Type::width>(-1) << size;
    return a & mask;
}

template<int SrcBitOffset, int OutBitSize, typename In>
inline ap_uint<OutBitSize>
extract(In const& in)
{
    assert(OutBitSize <= In::width - SrcBitOffset);
    return in(in.width - SrcBitOffset - 1, In::width - SrcBitOffset - OutBitSize);
}

template<int SrcBitOffset, typename In, typename Out>
inline void
extract(In const& in, Out & out)
{
    out = extract<SrcBitOffset, Out::width>(in);
}

template<int OutBitSize, typename In>
inline ap_uint<OutBitSize>
extract(std::size_t offset, In const& in)
{
    assert(offset < In::width);
    return extract<0, OutBitSize, In::width>(in << offset);
}

template<int BitOffset, typename In, typename Out>
inline void
insert(Out & out, In const& in)
{
    assert(Out::width - BitOffset >= In::width);
    out(out.width - BitOffset - 1, out.width - BitOffset - In::width) = in;
}

template<std::size_t MsbBitsFromA, typename Type>
inline Type
or_masked(Type const& a, Type const& b)
{
    return get_msb<MsbBitsFromA>(a), get_lsb<Type::width - MsbBitsFromA>(b);
}

template<typename Type>
inline Type
or_masked(Type const& a, Type const& b, std::size_t msb_bits_from_a)
{
    Type const a_mask = Type(-1) << (Type::width - msb_bits_from_a);
    return (a & a_mask) | (b >> msb_bits_from_a);
}

template<typename IntegerType>
inline IntegerType
align(IntegerType const& value, IntegerType const& alignment)
{
    assert(alignment & alignment - 1 == 0);

    IntegerType const mask = alignment - 1;
    return (value + mask) & mask;
}

template<typename Expected, typename Actual>
void
assert_eq(Expected const& expected, Actual const& actual,
          const char * file, std::size_t line)
{
    if (expected != actual)
    {
        std::cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
                  << file << ":" << line << " error:\n"
                  << "Expected:\n\t " << expected << "\n"
                  << "Got:\n\t " << actual
                  << std::endl;

//        std::abort();
    }
}

#define ASSERT_EQ(Expected, Actual) \
    ::enyx::assert_eq(Expected, Actual, __FILE__, __LINE__)

template <std::size_t x>
struct log2
{
    enum { value = 1 + log2<x/2>::value };
};

template <>
struct log2<1>
{
    enum { value = 0 };
};

template <>
struct log2<0>
{
    enum { value = 0 };
};

template <std::size_t x>
struct pow2
{
    enum { value = 2 * pow2<x-1>::value };
};

template <>
struct pow2<0>
{
    enum { value = 1 };
};

template<std::size_t a, std::size_t b>
struct min
{
    static std::size_t const value = a < b ? a : b;
};

template<std::size_t Integer>
struct itoa
{
    typedef itoa<Integer / 10> up;

    static std::size_t const value = (up::value << 8) + (Integer % 10) + '0';
    static std::size_t const byte_count = up::byte_count + 1;
};

template<>
struct itoa<0>
{
    static std::size_t const value = 0;
    static std::size_t const byte_count = 0;
};

template<std::size_t Power>
struct pow10_t
{
    static std::size_t const value = 10 * pow10_t<Power - 1>::value;
};

template<>
struct pow10_t<0>
{
    static std::size_t const value = 1;
};

template<int _AP_W, int _AP_I, ap_q_mode _AP_Q, ap_o_mode _AP_O, int _AP_N>
inline ap_fixed<_AP_W, _AP_I, _AP_Q, _AP_O, _AP_N>
ceil(ap_fixed<_AP_W, _AP_I, _AP_Q, _AP_O, _AP_N> x)
{
    if (x(_AP_W - _AP_I - 1, 0) != 0)
    {
        x(_AP_W - _AP_I - 1, 0) = 0;
        x += 1;
    }
    return x;
}

template<int _AP_W, int _AP_I, ap_q_mode _AP_Q, ap_o_mode _AP_O, int _AP_N>
inline ap_fixed<_AP_W, _AP_I, _AP_Q, _AP_O, _AP_N>
abs(ap_fixed<_AP_W, _AP_I, _AP_Q, _AP_O, _AP_N> f)
{
    if (f < 0)
        return -f;

    return f;
}
}
