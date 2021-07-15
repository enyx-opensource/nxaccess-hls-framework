/** @file
 *  @brief Contains Helper functions.
 *  @date 2021
 */

#pragma once

#include <memory>
#include <stdexcept>
#include <system_error>

#include <enyx/hw/a2c_stream.hpp>
#include <enyx/hw/c2a_stream.hpp>


namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

/**
 * @brief Find an accelerator from its index.
 *
 *
 * @param accelerator_index index of the accelerator
 * @return enyx::hw::accelerator accelerator found.
 * @throw runtime error if not found.
 */
enyx::hw::accelerator
find_accelerator(uint8_t accelerator_index);

/**
 * @brief Find a CPU to PPGA stream using it's usage.
 *
 * @param accelerator accelerator to use
 * @param usage Name of the stream.
 * @return enyx::hw::c2a_stream The CPU to FPGA stream
 * @throw runtime error if not found.
 */
enyx::hw::c2a_stream
find_c2a_stream(enyx::hw::accelerator& accelerator, const std::string & usage);

/**
 * @brief Find a PPGA to CPU stream using it's usage.
 *
 * @param accelerator accelerator to use
 * @param usage Name of the stream.
 * @return enyx::hw::a2c_stream The FPGA to CPU stream
 * @throw runtime error if not found.
 */
enyx::hw::a2c_stream
find_a2c_stream(enyx::hw::accelerator& accelerator, const std::string & usage);



inline
enyx::hw::accelerator
find_accelerator(uint8_t accelerator_index) {
    const enyx::hw::index index(accelerator_index);
    const enyx::hw::filter filter(index);
    const auto descriptors = enyx::hw::enumerate_accelerators(filter);
    if (descriptors.size() != 1) {
        throw std::runtime_error("Unable to find accelerator " + std::to_string(accelerator_index));
    }
    return enyx::hw::accelerator{descriptors.at(0)};
}

inline
enyx::hw::a2c_stream
find_a2c_stream(enyx::hw::accelerator& accelerator, const std::string & usage) {
    // Find and instantiate the CPU to accelerator stream
    const enyx::hw::name name(usage);
    const enyx::hw::filter filter(name);
    auto const descriptors = accelerator.enumerate_a2c_streams(filter);
    if (descriptors.size() != 1) {
        throw std::runtime_error("Unable to retrieve accelerator to cpu stream " + usage);
    }
    return enyx::hw::a2c_stream(descriptors.at(0));
}

inline
enyx::hw::c2a_stream
find_c2a_stream(enyx::hw::accelerator& accelerator, const std::string & usage) {
    // Find and instantiate the CPU to accelerator stream
    const enyx::hw::name name(usage);
    const enyx::hw::filter filter(name);
    auto const descriptors = accelerator.enumerate_c2a_streams(filter);
    if (descriptors.size() != 1) {
        throw std::runtime_error("Unable to retrieve cpu to accelerator stream " + usage);
    }
    return enyx::hw::c2a_stream(descriptors.at(0));
}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx