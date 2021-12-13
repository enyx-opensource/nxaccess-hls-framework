/** @file
 *  @brief Contains AlgorithmDriver.
 *  @date 2021
 */

#pragma once

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>

#include <enyx/utils/BufferView.hpp>
#include <enyx/hw/accelerator.hpp>
#include <enyx/hw/c2a_stream.hpp>
#include <enyx/hw/a2c_stream.hpp>

#include <enyx/oe/hwstrat/demo/Protocol.hpp>
#include <enyx/oe/hwstrat/demo/AlgorithmDispatcher.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

using DataView = enyx::utils::BufferView<const uint8_t>;

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
     */
    void
    poll();

    /**
     *  @brief Send configuration to FPGA
     *  @param update The update to apply.
     *  @return The status of the call.
     */
    std::error_code
    sendConfiguration(const InstrumentConfiguration & update);


    /**
     * @brief Trigger an collection using the sandbox with some arguments.
     *        Depending on the number of argument, the message sent will have a different size.
     *
     * @param collection_id Id of the collection to trigger
     * @param arg0 First argument of the trigger. Should be a valid buffer with a size between 1 and 16.
     * @param arg1 Second argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg2 Third argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg3 Forth argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg4 Fifth argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer.
     * @return std::error_code An error code.  User shall retry when error code is [ std::errc::resource_unavailable_try_again ].
     */
    std::error_code
    trigger(uint16_t collection_id,
            const DataView& arg0,
            const DataView& arg1 = {},
            const DataView& arg2 = {},
            const DataView& arg3 = {},
            const DataView& arg4 = {});

    /**
     * @brief Trigger an collection using the sandbox with some arguments.
     *        Depending on the number of argument, the message sent will have a different size.
     *
     * @param to_send binded argument message structure
     * @return std::error_code An error code.  User shall retry when error code is [ std::errc::resource_unavailable_try_again ].
     */
    std::error_code
    trigger(const TriggerWithArgsMessage& to_send);

private:
    enyx::hw::accelerator accelerator_;
    enyx::hw::c2a_stream c2a_stream_;
    enyx::hw::a2c_stream a2c_stream_;
    AlgorithmDispatcher dispatcher_;
    enyx::hw::a2c_stream::poller<std::reference_wrapper<AlgorithmDispatcher>> poller_;
};

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
