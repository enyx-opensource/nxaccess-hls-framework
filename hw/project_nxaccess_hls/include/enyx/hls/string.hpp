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

#include "helpers.hpp"
#include "byte.hpp"

namespace enyx {

template<typename Word>
std::string
text_from_word(Word const& word)
{
    std::ostringstream result;

    for (std::size_t i = 0; i != Word::width / byte::width; ++i)
        result << char(enyx::get_msb<byte::width>(word << i * byte::width));

    return result.str();
}

template<typename Word>
std::string
hex_from_word(Word const& word)
{
    std::ostringstream result;

    for (std::size_t i = 0; i != Word::width / byte::width; ++i)
        result << std::hex << std::setfill('0') << std::setw(2)
               << uint16_t(enyx::get_msb<byte::width>(word << i * byte::width));

    return result.str();
}

template<typename Word>
Word
word_from_text(std::string const& content)
{
    static std::size_t const word_byte_count = Word::width / byte::width;
    assert(content.size() <= word_byte_count);

    Word word = 0;
    for (std::size_t i = 0, e = content.size(); i != e; ++i)
    {
        word <<= 8;
        word(7, 0) = content[i];
    }

    // Move text to MSB.
    word <<= byte::width * (word_byte_count - content.size());

    return word;
}

template<typename Word>
void
fill_word_from_text(Word & word, std::string const& content)
{
    word = word_from_text<Word>(content);
}

template<typename Word>
void
fill_stream_with_text(hls::stream<Word> & in, std::string const& content)
{
    static std::size_t const word_byte_count = Word::data_width / 8;

    Word word_in = Word();
    for (std::size_t i = 0, e = content.size(); i != e; )
    {
        word_in.data <<= 8;
        word_in.data(7, 0) = content[i];

        ++i;
        if (i % word_byte_count == 0)
        {
            word_in.last = i == content.size() - 1;
            in.write(word_in);
            word_in.data = 0;
        }
    }

    std::size_t const remaining_byte_count = content.size() % word_byte_count;
    if (remaining_byte_count)
    {
        word_in.data <<= (word_byte_count - remaining_byte_count) * 8;
        word_in.last = 1;
        in.write(word_in);
    }
}

template<typename Type>
inline Type
get_from_stream_as(std::istringstream & in)
{
    Type value;
    in >> value;
    return value;
}


/// reads hex data into target templated type.
template<typename Type>
inline Type
get_from_hex_stream_as(std::istringstream & in)
{
    Type value ;
    in >> std::hex >> value; 
    return value;
}
}