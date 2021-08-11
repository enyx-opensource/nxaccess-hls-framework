#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <system_error>
#include <vector>

#include <enyx/utils/StringUtils.hpp>
#include <enyx/utils/data_format/HexString.hpp>

#include <enyx/oe/hwstrat/demo/Helper.hpp>


namespace enyx {
namespace tools {

using Buffer = std::vector<uint8_t>;
using Buffers = std::vector<Buffer>;


inline
Buffer
parse_simu_file_line(const std::string& line, std::size_t line_number, const std::string& filename) {
    auto trimed = utils::trim(line);
    // Handle empty / commented lines
    if (trimed.empty() or utils::startsWith(trimed, "#")) {
        return {};
    }
    // Remove all spaces from the line
    trimed.erase(std::remove_if(trimed.begin(), trimed.end(), [] (char c){ return std::isspace(c); }),
                 trimed.end());
    // The - character means "don't care", it will be replace by 0
    std::replace(trimed.begin(), trimed.end(), '-', '0');
    // Convert the hex string to binary.
    try {
        return utils::data_format::hexStringToBin(trimed);
    } catch (const std::exception& e) {
        std::cerr << "Exception caught while parsing file '"
            << filename << "' at line " << line_number
            << ": " << e.what() << "\n";
        return {};
    }
}

inline
void
parse_simu_file(Buffers& ret, const std::string& filename) {
    std::ifstream file(filename);
    if (not file.is_open()) {
        std::cerr << "Unable to open file: '" + filename + "'\n";
        return;
    }
    std::size_t line_number = 1;
    for (std::string line; std::getline(file, line);) {
        ret.emplace_back(parse_simu_file_line(line, line_number++, filename));
    }
}

template <typename IteratorT>
Buffers
parse_simu_files(IteratorT begin, IteratorT end) {
    Buffers ret;
    for (; begin != end; ++begin) {
        parse_simu_file(ret, *begin);
    }
    return ret;
}

inline
void
inject(std::size_t accelerator_id, const std::string& stream_name, const Buffers& to_inject) {
    namespace demo = enyx::oe::hwstrat::demo;
    auto accelerator = demo::find_accelerator(accelerator_id);
    auto c2a_stream = demo::find_c2a_stream(accelerator, stream_name);
    for (const auto& buffer: to_inject) {
        if (buffer.empty()) { continue; }
        const auto send_fn = [&c2a_stream, &buffer] () {
                return c2a_stream.send(buffer.data(), buffer.size()).error(); };
        const auto send_error = demo::retry_on_eagain_with_timeout(send_fn);
        if (send_error) {
            throw std::runtime_error("Unable to send data: " + send_error.message());
        }
    }
}

} // namespace tools
} // namespace enyx
