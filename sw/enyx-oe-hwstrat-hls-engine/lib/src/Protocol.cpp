#include <enyx/oe/hwstrat/demo/Protocol.hpp>

#include <ostream>

#include <endian.h>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {


std::ostream&
operator<<(std::ostream& os, const FpgaToCpuHeader& v) {
    os  << "[ "
        << " version: " << uint32_t(v.version)
        << " source: " << uint32_t(v.source)
        << " msg_type: " << uint32_t(v.msg_type)
        << " error: " << uint32_t(v.error)
        << " timestamp: " << be32toh(v.timestamp)
        << " length: " << be16toh(v.length)
        << " ]";
    return os;
}


std::ostream&
operator<<(std::ostream& os, const InstrumentConfigurationMessage&) {
    //TODO
    return os;
}


std::ostream&
operator<<(std::ostream& os, const InstrumentConfigurationAckMessage& v) {
    os << v.header <<  v.configuration;
    return os;
}

std::ostream&
operator<<(std::ostream& os, const TickToCancelNotificationMessage& v) {
    os << v.header
       <<  " trade_summary_price:" << be64toh(v.trade_summary_price)
       <<  " book_top_level_price:" << be64toh(v.book_top_level_price)
       <<  " threshold:" << be64toh(v.threshold)
       <<  " instrument_id:" << be32toh(v.instrument_id)
       <<  " sent_collection_id:" << be16toh(v.sent_collection_id)
       <<  " is_bid:" << uint32_t(v.is_bid);
    return os;
}


std::ostream&
operator<<(std::ostream& os, const TickToTradeNotificationMessage& v) {
    os << v.header
       <<  " trade_summary_price:" << be64toh(v.trade_summary_price)
       <<  " threshold_price:" << be64toh(v.threshold_price)
       <<  " instrument_id:" << be32toh(v.instrument_id)
       <<  " sent_collection_id:" << be16toh(v.sent_collection_id)
       <<  " is_bid:" << uint32_t(v.is_bid);
    return os;
}

std::ostream&
operator<<(std::ostream& os, const InstrumentConfiguration& v) {
    os << "t2c_threshold:" << be64toh(v.price_threshold)
       << " t2t_bid_price:" << be64toh(v.bid_price)
       << " t2t_ask_price:" << be64toh(v.ask_price)
       << " instrument_id:" << be32toh(v.instrument_id)
       << " t2t_bid_collection_id:" << be16toh(v.tick_to_trade_bid_collection_id)
       << " t2c_collection_id:" << be16toh(v.tick_to_cancel_collection_id)
       << " t2t_ask_collection_id: " << be16toh(v.tick_to_trade_ask_collection_id)
       << " enabled: " << int(v.enabled);
    return os;
}

} // demo namespace
} // hwstrat namespace

} // namespace oe
} // namespace enyx
