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
#include <enyx/utils/log/macros.hpp>

#define _BSD_SOURCE
#include <endian.h>

#include <enyx/oe/hwstrat/demo/ErrorCode.hpp>
#include <enyx/oe/hwstrat/demo/Helper.hpp>
#include <enyx/oe/hwstrat/demo/Handler.hpp>
#include <enyx/oe/hwstrat/demo/StandAloneTrigger.hpp>



namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

std::string get_hex_string(const std::uint8_t * data, std::uint32_t size ) {
    std::stringstream out;
    for(std::size_t i = 0 ; i < size; ++i){
        out << std::hex << std::setfill('0') << std::setw(2) << uint32_t(data[i]);
    }

    return out.str();
}

template<typename MessageType>
std::string get_hex_string(MessageType const& message) {
    const uint8_t * data = reinterpret_cast<const uint8_t*>(&message);
    return get_hex_string(data, sizeof(MessageType));
}


namespace {

constexpr std::uint32_t MMDeviceId = 0;
const char * LogPrefix = "AlgorithmDriver";
const char * HfpChannelUsage = "user0";

template<typename Setting>
std::error_code
sendToFpga(enyx::hw::c2a_stream & stream,
           Setting const& setting) {
    // This log can potentially impact performance, so please remove it for production use.
    LOG_ME(NX_INFO, "[%s] Raw content of message to be sent: %s",
         LogPrefix, get_hex_string(setting).c_str());

    //TODO: stop condition
    std::error_code error;
    do {
        error = stream.send(&setting, sizeof(setting)).error();
    } while (error == std::errc::resource_unavailable_try_again);

    return error;
}

const uint8_t APPLICATION_VERSION = 1;

} // namespace

struct AlgorithmDriver::Impl {
    Impl(std::uint16_t boardId,
         Handler & handler)
            : boardId_(boardId),
              handler_(handler) {
    }
    ~Impl() = default;

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    void
    operator()(const std::uint8_t * data, std::uint32_t size) {
        using HeaderType = FpgaToCpuHeader;

        if (size <= sizeof(HeaderType)) {
            handler_.onError(make_error_code(CORRUPTED_APPLICATION_HEADER));
            return;
        }

        // This log can potentially impact performance, so please remove it for production use.
        LOG_ME(NX_INFO, "[%s] Raw content of received message : %s",
             LogPrefix, get_hex_string(data, size).c_str());

        const auto * header = reinterpret_cast<const HeaderType*>(data);

        // Sanity check
        if (be16toh(header->length) != size
                || header->version != APPLICATION_VERSION) {
            LOG_ME(NX_CRITICAL, "[%s] Corrupted application header: version: %d"
                " length: %d  buffer size: %d",
                 LogPrefix, header->version, header->length, size);
            handler_.onError(make_error_code(CORRUPTED_APPLICATION_HEADER));
            return;
        }

        switch (static_cast<ModulesIds>(header->source)) {
            case ModulesIds::InstrumentDataConfiguration:
                handler_.on(*reinterpret_cast<const InstrumentConfigurationAckMessage*>(data));
                return;
            case ModulesIds::SoftwareTrigger:
                LOG_ME(NX_CRITICAL, "[%s] Received unexpected Software Trigger message", LogPrefix);
                handler_.onError(make_error_code(UNKNOWN_ALGORITHM_MESSAGE));
                return;
            case ModulesIds::TickToCancel:
                handler_.on(*reinterpret_cast<const TickToCancelNotificationMessage*>(data));
                return;
            case ModulesIds::TickToTrade:
                handler_.on(*reinterpret_cast<const TickToTradeNotificationMessage*>(data));
                return;
        }
        LOG_ME(NX_CRITICAL, "[%s] Message received with unknown source: %d", LogPrefix, header->source);
        handler_.onError(make_error_code(UNKNOWN_ALGORITHM_MESSAGE));

    }

    const uint16_t boardId_;
    enyx::hw::accelerator accelerator_{find_accelerator(boardId_)};
    enyx::hw::c2a_stream c2aStream_{find_c2a_stream(accelerator_, HfpChannelUsage)};
    enyx::hfp::mm mm_{boardId_, MMDeviceId};
    enyx::hfp::rx rx_{boardId_, HfpChannelUsage};
    enyx::hfp::rx::poller<std::reference_wrapper<Impl>> rxPoller_{rx_.get_poller(std::ref(*this))};
    Handler & handler_;
};

AlgorithmDriver::AlgorithmDriver(Handler & handler,
                                 std::uint16_t boardIndex)
    : impl_(new Impl{boardIndex, std::ref(handler)}) {
}

AlgorithmDriver::~AlgorithmDriver() = default;

std::error_code
AlgorithmDriver::poll() {
    assert(impl_);
    impl_->rxPoller_.poll();
    return std::error_code{};
}

std::error_code
AlgorithmDriver::sendConfiguration(const InstrumentConfiguration & conf) {
    assert(impl_);
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

    return sendToFpga(impl_->c2aStream_, update);
}

std::error_code
AlgorithmDriver::trigger(uint16_t collection_id,
                         const utils::BufferView<const uint8_t>& arg0,
                         const utils::BufferView<const uint8_t>& arg1,
                         const utils::BufferView<const uint8_t>& arg2,
                         const utils::BufferView<const uint8_t>& arg3,
                         const utils::BufferView<const uint8_t>& arg4) {
    assert(impl_);
    return StandAloneTrigger::trigger_helper(impl_->c2aStream_, collection_id, arg0, arg1, arg2, arg3, arg4);
}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
