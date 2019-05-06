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

#include <hls_stream.h>
#include <ap_int.h>
#include <utils/x_hls_utils.h>
#include "byte.hpp"

namespace enyx {

using namespace hls;

template<typename Word>
struct word_traits
{
    static std::size_t const data_width = Word::data_width;
};

template<int Width>
struct word_traits<ap_uint<Width> >
{
    static std::size_t const data_width = Width;
};

template<typename Word>
inline ap_uint<word_traits<Word>::data_width> const&
get_word_data(Word const& word)
{
    return word.data;
}

template<int Width>
inline ap_uint<Width> const&
get_word_data(ap_uint<Width> const& word)
{
    return word;
}

template<typename Word>
struct has_last
{
    template<int>
    struct exist;

    template<typename T>
    static char (&test(exist<sizeof(&T::last)>*))[1];

    template<typename T>
    static char (&test(...))[2];

    static bool const value = sizeof(test<Word>(0)) == 1;
};

template<typename Word>
inline bool
is_last(Word const& word, integral_constant<bool, true>)
{
    return word.last;
}

template<typename Word>
inline bool
is_last(Word const& word, integral_constant<bool, false>)
{
    return true;
}

template<typename Word>
inline bool
is_last(Word const& word)
{
    return is_last(word, integral_constant<bool, has_last<Word>::value>());
}
} // Namespace