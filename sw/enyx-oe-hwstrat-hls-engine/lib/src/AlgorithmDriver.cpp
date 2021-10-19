#include <enyx/oe/hwstrat/demo/AlgorithmDriver.hpp>

#include <endian.h>

#include <functional>
#include <type_traits>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

#include <enyx/hfp/mm.hpp>
#include <enyx/hfp/rx.hpp>
#include <enyx/hfp/tx.hpp>
#include <enyx/hfp/error.hpp>
#include <enyx/utils/data_format/HexString.hpp>
#include <enyx/utils/log/macros.hpp>

#include <enyx/oe/hwstrat/demo/ErrorCode.hpp>
#include <enyx/oe/hwstrat/demo/Helper.hpp>
#include <enyx/oe/hwstrat/demo/Handler.hpp>
#include <enyx/oe/hwstrat/demo/StandAloneTrigger.hpp>



namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

namespace {

const char * LogPrefix = "AlgorithmDriver";
const char * HfpChannelUsage = "user0";

template<typename MessageType>
std::error_code
sendToFpga(enyx::hw::c2a_stream & stream,
           MessageType const& message) {

    const size_t length = message.header.length;
    const uint8_t * const data = reinterpret_cast<const std::uint8_t *>(&message);
    // This log can potentially impact performance, so please remove it for production use.
    LOG_ME(NX_DEBUG, "[%s] Raw content of message to be sent: %s",
         LogPrefix, enyx::utils::data_format::toHexString({data, length}).c_str());

    const auto send_fn = [&stream, data, length] () {
            return stream.send(data, length).error(); };

    return retry_on_eagain_with_timeout(send_fn);
}

} // namespace

AlgorithmDriver::AlgorithmDriver(Handler & handler,
                                 std::uint16_t boardIndex)
        : accelerator_(find_accelerator(boardIndex))
        , c2a_stream_(find_c2a_stream(accelerator_,HfpChannelUsage))
        , a2c_stream_(demo::find_a2c_stream(accelerator_, HfpChannelUsage))
        , dispatcher_(handler)
        , poller_(a2c_stream_.get_poller(std::ref(dispatcher_)))
    {}

AlgorithmDriver::~AlgorithmDriver() = default;

void
AlgorithmDriver::poll() {

    poller_.poll_once();
}

std::error_code
AlgorithmDriver::sendConfiguration(const InstrumentConfiguration & conf) {

    InstrumentConfigurationMessage update;
    // Header
    update.header.version = APPLICATION_VERSION;
    update.header.dest = static_cast<uint8_t>(ModulesIds::InstrumentDataConfiguration);
    update.header.msg_type = 1;  // hardware filters on it.
    update.header.ack_request = 1; // not implemented in hardware
    update.header.timestamp = 0x12345678; // dumb value for timestamping.
    update.header.length = sizeof(update);

    //body
    update.configuration = conf;

    return sendToFpga(c2a_stream_, update);
}

std::error_code
AlgorithmDriver::trigger(const TriggerWithArgsMessage& to_send) {

    return sendToFpga(c2a_stream_, to_send);
}

std::error_code
AlgorithmDriver::trigger(uint16_t collection_id,
                         const utils::BufferView<const uint8_t>& arg0,
                         const utils::BufferView<const uint8_t>& arg1,
                         const utils::BufferView<const uint8_t>& arg2,
                         const utils::BufferView<const uint8_t>& arg3,
                         const utils::BufferView<const uint8_t>& arg4) {

    TriggerWithArgsMessage to_send;
    const bool binded = StandAloneTrigger::bind_arguments(to_send, collection_id, arg0, arg1, arg2, arg3, arg4);
    if(not binded) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    return trigger(to_send);
}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
