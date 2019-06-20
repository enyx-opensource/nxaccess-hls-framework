/** @file
 *  @brief Contains AlgorithmDriver.
 *  @date 2019
 */

#pragma once

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>

#include <enyx/oe/hwstrat/demo/Protocol.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

struct Handler;

/**
 *  @brief This class allows user to communication with the HLS algorithm.
 */
class AlgorithmDriver {
public:
    /**
     *  @brief Prevent copy.
     */
    AlgorithmDriver(const AlgorithmDriver &) = delete;

    /**
     *  @brief Prevent copy.
     *  @return A reference to this.
     */
    AlgorithmDriver &
    operator=(const AlgorithmDriver &) = delete;

    /**
     *  @brief Construct an algorithm driver.
     *  @param handler The handler called on each event from the algorithm.
     *  @param boardIndex The board executing the HLS algorithm
     */
    explicit
    AlgorithmDriver(Handler & handler,
                    std::uint16_t boardIndex = 0);


    /**
     *  @brief Destroy the algorithm instance.
     */
    ~AlgorithmDriver();

    /**
     *  @brief Repeatedly called to process events.
     *  @note For maximum performance, the thread calling this function
     *        should be pinned to a dedicated CPU, isolated from scheduler.
     *  @return The error code
     */
    std::error_code
    poll();

    /**
     *  @brief Send configuration to FPGA
     *  @param update The update to apply.
     *  @return The status of the call.
     */
    std::error_code
    sendConfiguration(const InstrumentConfiguration & update);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx