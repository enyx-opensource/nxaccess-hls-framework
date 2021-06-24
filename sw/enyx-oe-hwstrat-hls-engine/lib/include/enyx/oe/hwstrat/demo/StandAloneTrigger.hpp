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

#include <enyx/oe/hwstrat/demo/Helper.hpp>
#include <enyx/oe/hwstrat/demo/Protocol.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

/**
 * @brief Class to trigger a collection with arg using the hls demo firmware.
 *        This class cannot be instantiated with AlgorithmDriver since they both will try
 *        to uniquely acquire the sandbox cpu2fpga steam.
 */
class StandAloneTrigger {
public:

    using ArgType = utils::BufferView<const uint8_t>;

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
            const ArgType& arg0,
            const ArgType& arg1 = {},
            const ArgType& arg2 = {},
            const ArgType& arg3 = {},
            const ArgType& arg4 = {});

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
                   const ArgType& arg0,
                   const ArgType& arg1 = {},
                   const ArgType& arg2 = {},
                   const ArgType& arg3 = {},
                   const ArgType& arg4 = {});

private:
    enyx::hw::accelerator accelerator_;
    enyx::hw::c2a_stream stream_{find_c2a_stream(accelerator_, "user0")};
};


inline
StandAloneTrigger::StandAloneTrigger(uint8_t accelerator_index)
        : accelerator_(find_accelerator(accelerator_index)) {
}

inline
std::error_code
StandAloneTrigger::trigger(uint16_t collection_id,
            const ArgType& arg0,
            const ArgType& arg1,
            const ArgType& arg2,
            const ArgType& arg3,
            const ArgType& arg4) {
    return trigger_helper(stream_, collection_id, arg0, arg1, arg2, arg3, arg4);
}


inline
std::error_code
StandAloneTrigger::trigger_helper(enyx::hw::c2a_stream& stream,
            uint16_t collection_id,
            const ArgType& arg0,
            const ArgType& arg1,
            const ArgType& arg2,
            const ArgType& arg3,
            const ArgType& arg4) {
    TriggerWithArgsMessage to_send;
    // Fill header
    to_send.header.dest = static_cast<uint8_t>(ModulesIds::SoftwareTrigger);
    to_send.header.version = 1;
    to_send.header.ack_request = 1;
    to_send.header.msg_type= 1;
    to_send.header.length = sizeof(CpuToFpgaHeader) + sizeof(TriggerWithArgsHeader);
    // Fill Trigger
    to_send.trigger.collectionId = ((collection_id >> 8) &0xFF) |  ((collection_id & 0xFF) << 8) ;
    to_send.trigger.argBitmap = 1;

    // Fill argument
    uint16_t args_size = 16;
    if (arg0.size() == 0 or arg0.size() > to_send.args.arg0.size()) {
        return std::make_error_code(std::errc::invalid_argument);
    }
    std::copy(arg0.cbegin(), arg0.cend(), to_send.args.arg0.begin());
    if (arg1.size() != 0) {
        if (arg1.size() > to_send.args.arg1.size()) {
            return std::make_error_code(std::errc::invalid_argument);
        }
        args_size = 2 * 16;
        to_send.trigger.argBitmap |= 1 << 1;
        std::copy(arg1.cbegin(), arg1.cend(), to_send.args.arg1.begin());
    }
    if (arg2.size() != 0) {
        if (arg2.size() > to_send.args.arg2.size()) {
            return std::make_error_code(std::errc::invalid_argument);
        }
        args_size = 3 * 16;
        to_send.trigger.argBitmap |= 1 << 2;
        std::copy(arg2.cbegin(), arg2.cend(), to_send.args.arg2.begin());
    }
    if (arg3.size() != 0) {
        if (arg3.size() > to_send.args.arg3.size()) {
            return std::make_error_code(std::errc::invalid_argument);
        }
        args_size = 4 * 16;
        to_send.trigger.argBitmap |= 1 << 3;
        std::copy(arg3.cbegin(), arg3.cend(), to_send.args.arg3.begin());
    }
    if (arg4.size() != 0) {
        if (arg4.size() > to_send.args.arg4.size()) {
            return std::make_error_code(std::errc::invalid_argument);
        }
        args_size = 5 * 16;
        to_send.trigger.argBitmap |= 1 << 4;
        std::copy(arg4.cbegin(), arg4.cend(), to_send.args.arg4.begin());
    }
    // TODO: Due to temporary limitation in the hardware implementation,always send all values.
    args_size = 5 * 16;

    to_send.header.length += args_size;
    return stream.send(reinterpret_cast<const void*>(&to_send), to_send.header.length).error();
}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx