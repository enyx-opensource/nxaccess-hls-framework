#include <enyx/oe/hwstrat/demo/Protocol.hpp>

#include <ostream>

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
        << " timestamp: " << v.timestamp
        << " length: " << uint32_t(v.length)
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
    os << v.header
       << " enabled: " <<  uint32_t(v.enabled)
       << " instrument_id: " << v.instrument_id
       << " tick_to_cancel_threshold: " << v.tick_to_cancel_threshold
       << " tick_to_cancel_collection_id: " << v.tick_to_cancel_collection_id
       << " tick_to_trade_bid_price: " << v.tick_to_trade_bid_price
       << " tick_to_trade_bid_collection_id: " << v.tick_to_trade_bid_collection_id
       << " tick_to_trade_ask_price: " << v.tick_to_trade_ask_price
       << " tick_to_trade_ask_collection_id: " << v.tick_to_trade_ask_collection_id;
    return os;
}

std::ostream&
operator<<(std::ostream& os, const TickToCancelNotificationMessage& v) {
    os << v.header
       <<  " trade_summary_price:" << v.trade_summary_price
       <<  " book_top_level_price:" << v.book_top_level_price
       <<  " threshold" << v.threshold
       <<  " instrument_id" << v.instrument_id
       <<  " sent_collection_id:" << v.sent_collection_id
       <<  " is_bid:" << v.is_bid;
    return os;
}


std::ostream&
operator<<(std::ostream& os, const TickToTradeNotificationMessage& v) {
    os << v.header
       <<  " trade_summary_price:" << v.trade_summary_price
       <<  " threshold_price:" << v.threshold_price
       <<  " instrument_id" << v.instrument_id
       <<  " sent_collection_id:" << v.sent_collection_id
       <<  " is_bid:" << uint32_t(v.is_bid);
    return os;
}

std::ostream&
operator<<(std::ostream& os, const InstrumentConfiguration& v) {
    os << "t2c_threshold: " << v.tick_to_cancel_threshold
       << " t2t_bid_price: " << v.tick_to_trade_bid_price
       << " t2t_ask_price: " << v.tick_to_trade_ask_price
       << " instrument_id: " << v.instrument_id
       << " t2t_bid_collection_id: " << v.tick_to_trade_bid_collection_id
       << " t2c_collection_id: " << v.tick_to_cancel_collection_id
       << " t2t_ask_collection_id: " << v.tick_to_trade_ask_collection_id
       << " enabled: " << int(v.enabled);
    return os;
}

} // demo namespace
} // hwstrat namespace

} // namespace oe
} // namespace enyx
