#include <enyx/oe/hwstrat/demo/AlgorithmDispatcher.hpp>

#include <iomanip>

#include <enyx/utils/data_format/HexString.hpp>
#include <enyx/utils/log/macros.hpp>

#include <enyx/oe/hwstrat/demo/ErrorCode.hpp>
#include <enyx/oe/hwstrat/demo/Handler.hpp>
#include <enyx/oe/hwstrat/demo/Helper.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

void
AlgorithmDispatcher::operator()(const uint8_t * data, uint32_t size) {
    using HeaderType = FpgaToCpuHeader;

    if (size <= sizeof(HeaderType)) {
        handler_.onError(make_error_code(CORRUPTED_APPLICATION_HEADER));
        return;
    }

    // This log can potentially impact performance, so please remove it for production use.
    LOG_ME(NX_INFO, "[AlgorithmDispatcher] Raw content of received message : %s",
           enyx::utils::data_format::toHexString({data, size}).c_str());

    const auto * header = reinterpret_cast<const HeaderType*>(data);

    // Sanity check
    if (be16toh(header->length) != size
            || header->version != APPLICATION_VERSION) {
        LOG_ME(NX_CRITICAL, "[AlgorithmDispatcher] Corrupted application header: version: %d"
            " length: %d  buffer size: %d", header->version, header->length, size);
        handler_.onError(make_error_code(CORRUPTED_APPLICATION_HEADER));
        return;
    }

    switch (static_cast<ModulesIds>(header->source)) {
        case ModulesIds::InstrumentDataConfiguration:
            handler_.on(*reinterpret_cast<const InstrumentConfigurationAckMessage*>(data));
            return;
        case ModulesIds::SoftwareTrigger:
            LOG_ME(NX_CRITICAL, "[AlgorithmDispatcher] Received unexpected Software Trigger message");
            handler_.onError(make_error_code(UNKNOWN_ALGORITHM_MESSAGE));
            return;
        case ModulesIds::TickToCancel:
            handler_.on(*reinterpret_cast<const TickToCancelNotificationMessage*>(data));
            return;
        case ModulesIds::TickToTrade:
            handler_.on(*reinterpret_cast<const TickToTradeNotificationMessage*>(data));
            return;
    }
    LOG_ME(NX_CRITICAL, "[AlgorithmDispatcher] Message received with unknown source: %d", header->source);
    handler_.onError(make_error_code(UNKNOWN_ALGORITHM_MESSAGE));

}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
