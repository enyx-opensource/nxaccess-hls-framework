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


template <typename Functor>
inline
std::error_code
retry_with_timeout(Functor&& fn,
                  const std::chrono::milliseconds& timeout = std::chrono::milliseconds{100}) {
    constexpr auto eagain = std::errc::resource_unavailable_try_again;
    const auto now = std::chrono::steady_clock::now;
    // Small optimisation to not call now() if it works right away.
    auto ret = fn();
    if (ret != eagain) {
        return ret;
    }
    const auto start = now();
    while ((ret == eagain) and (now() - start < timeout)) {
        ret = fn();
    }
    return ret;
}


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
Buffers
parse_simu_file(const std::string& filename) {
    std::ifstream file(filename);
    if (not file.is_open()) {
        std::cerr << "Unable to open file: '" + filename + "'\n";
        return {};
    }
    std::size_t line_number = 1;
    Buffers ret;
    for (std::string line; std::getline(file, line);) {
        ret.emplace_back(parse_simu_file_line(line, line_number++, filename));
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
        const auto send_ret = retry_with_timeout(send_fn);
        if (send_ret) {
            throw std::runtime_error("Unable to send data: " + send_ret.message());
        }
    }
}

} // namespace tools
} // namespace enyx