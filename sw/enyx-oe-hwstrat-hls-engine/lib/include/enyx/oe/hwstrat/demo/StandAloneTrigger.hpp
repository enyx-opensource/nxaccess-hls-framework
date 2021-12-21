/** @file
 *  @brief Contains StandAloneTrigger.
 *  @date 2021
 */

#pragma once

#include <memory>
#include <stdexcept>
#include <system_error>

#include <enyx/utils/BufferView.hpp>
#include <enyx/hw/c2a_stream.hpp>

#include <enyx/oe/hwstrat/demo/Protocol.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

using DataView = enyx::utils::BufferView<const uint8_t>;
using DataArgViews = std::array<DataView, TRIGGER_NB_ARG>;

/**
 * @brief Class to trigger a collection with arg using the hls demo firmware.
 *        This class cannot be instantiated with AlgorithmDriver since they both will try
 *        to uniquely acquire the sandbox cpu2fpga steam.
 */
class StandAloneTrigger {
public:

    /**
     * @brief Construct a new Stand Alone Trigger object
     *
     * @param accelerator_index index of the accelerator to use.
     */
    StandAloneTrigger(uint8_t accelerator_index);

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

    /** @brief same as trigger using array of args */
    std::error_code
    trigger(uint16_t collection_id, const DataArgViews& args);

    /**
     * @brief Static method to trigger an collection using the sandbox with some arguments.
     *        Depending on the number of argument, the message sent will have a different size.
     *
     * @param stream stream to use to send the trigger.
     * @param collection_id Id of the collection to trigger
     * @param arg0 First argument of the trigger. Should be a valid buffer with a size between 1 and 16.
     * @param arg1 Second argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg2 Third argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg3 Forth argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer
     * @param arg4 Fifth argument of the trigger. Should be a buffer with a size between 1 and 16 or an empty buffer.
     * @return std::error_code An error code.  User shall retry when error code is [ std::errc::resource_unavailable_try_again ].
     */
    static std::error_code
    trigger_helper(enyx::hw::c2a_stream& stream,
                   uint16_t collection_id,
                   const DataView& arg0,
                   const DataView& arg1 = {},
                   const DataView& arg2 = {},
                   const DataView& arg3 = {},
                   const DataView& arg4 = {});

    /** @brief same as trigger_helper using array of args */
    static std::error_code
    trigger_helper(enyx::hw::c2a_stream& stream, uint16_t collection_id, const DataArgViews& args);

    static bool
    bind_arguments(TriggerWithArgsMessage& out,
        uint16_t collection_id,
        const DataView& arg0,
        const DataView& arg1 = {},
        const DataView& arg2 = {},
        const DataView& arg3 = {},
        const DataView& arg4 = {});

    /** @brief same as bind_arguments using array of args */
    static bool
    bind_arguments(TriggerWithArgsMessage& trigger_msg, uint16_t collection_id, const DataArgViews& args);

private:
    enyx::hw::accelerator accelerator_;
    enyx::hw::c2a_stream stream_;
};


} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
