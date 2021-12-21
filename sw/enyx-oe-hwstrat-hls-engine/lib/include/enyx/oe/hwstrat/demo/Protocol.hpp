/** @file
 *  @brief Contains Prorocol related class and functions.
 *  @date 2019
 */
#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>

#include <enyx/utils/compiler-dependencies.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

constexpr uint16_t UNALLOCATED_ID = UINT16_MAX;
constexpr uint8_t DEFAULT_ACK = 1; // Acknowledge requested
constexpr size_t TRIGGER_ARG_SIZE = 16;
constexpr size_t TRIGGER_NB_ARG = 5;
constexpr size_t MAX_INSTR = 256;
constexpr uint8_t APPLICATION_VERSION = 1;

using TriggerArg = std::array<uint8_t, TRIGGER_ARG_SIZE>;
using TriggerArgs = std::array<TriggerArg, TRIGGER_NB_ARG>;

enum class ModulesIds : uint8_t {
    InstrumentDataConfiguration = 8, // Module that handle instrument configuration, see configuration.hpp
    SoftwareTrigger = 9, // Not implemented yet, reserved for module handling trigger from software. // Not present in demonstration
    TickToCancel = 10,   // tick2cancel strategy
    TickToTrade = 11 // tick2trade strategy
};

/// CPU To FPGA header
struct ENYX_PACKED_STRUCT CpuToFpgaHeader {
    // first byte
    uint8_t dest:4;        /// source fpga module id use enum ModulesIds
    uint8_t version:4;     /// message format version
    // second byte
    uint8_t reserved:3;    ///
    uint8_t ack_request:1; /// Ack message request (1 = want ack)
    uint8_t msg_type:4;    /// message_type, defined per module

    uint32_t timestamp;    /// future use : hw timestamp, unused by soft
    uint16_t length;       /// message format version
};
static_assert(sizeof(CpuToFpgaHeader) == 8, "Invalid CpuToFpgaHeader size");

/**
 * @brief build default cpu to fpga header message of an instrument.
 */
template <typename T>
CpuToFpgaHeader buildCpuToFpgaHeader(ModulesIds module) {
    return {
        /*dest:4       */static_cast<uint8_t>(module),
        /*version:4    */APPLICATION_VERSION,
        /*reserved:3   */1,
        /*ack_request:1*/DEFAULT_ACK,
        /*msg_type:4   */1,
        /*timestamp    */0, // unused now
        /*length       */sizeof(T)
    };
}

struct ENYX_PACKED_STRUCT FpgaToCpuHeader {
    // first byte
    uint8_t source:4;      /// source fpga module id use enum ModulesIds
    uint8_t version:4;     /// message format version
    // second byte
    uint8_t reserved:3;    /// ack
    uint8_t error:1;       /// error bit in case message is not processed correctly
    uint8_t msg_type:4;    /// message_type

    uint32_t timestamp;    /// future use : hw timestamp, unused for now.
    uint16_t length;       /// message format version
};
static_assert(sizeof(FpgaToCpuHeader) == 8, "Invalid FpgaToCpuHeader size");


/**
 * @brief Configuration of an instrument.
 */
struct ENYX_PACKED_STRUCT InstrumentConfiguration {
    int64_t price_threshold; // price for tick 2 cancel
    int64_t bid_price; // price for tick 2 trade
    int64_t ask_price; // price for tick 2 trade
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t tick_to_trade_bid_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_cancel_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_trade_ask_collection_id; // collection id to trigger if price under threshold
    uint8_t enabled; /// Whether the configuration fot this instrument is enabled or not.
};


struct ENYX_PACKED_STRUCT InstrumentConfigurationMessage {
    struct CpuToFpgaHeader header = buildCpuToFpgaHeader<InstrumentConfigurationMessage>(ModulesIds::InstrumentDataConfiguration); // 64 bits
    InstrumentConfiguration configuration;
    std::array<uint8_t, 5> reserved; //ensure aligned on 128bits words
};
static_assert(sizeof(InstrumentConfigurationMessage) == 48, "Invalid InstrumentConfigurationMessage size");

/**
 * @brief Message to send to trigger a collection with args.
 *        The size of the message will vary depending on the arg_bitmap.
 */
struct ENYX_PACKED_STRUCT TriggerWithArgsMessage {
    CpuToFpgaHeader       header = buildCpuToFpgaHeader<TriggerWithArgsMessage>(ModulesIds::SoftwareTrigger);
    uint16_t collection_id { UINT16_MAX };/// Collection Id
    uint8_t arg_bitmap { 0 };             /// The arguments that will be present in this trigger request. To be mapped directly on the arg_valid bus.
    std::array<uint8_t, 5> reserved {{}}; /// Padding
    TriggerArgs args {{}};                /// only 5 argument can be triggered
};
static_assert(sizeof(TriggerWithArgsMessage) == 96, "Invalid TriggerWithArgsMessage size");

struct ENYX_PACKED_STRUCT InstrumentConfigurationAckMessage {
    struct FpgaToCpuHeader header; //version == 1, msgtype == 1, length ==
    InstrumentConfiguration configuration;
    std::array<uint8_t, 5> reserved; //ensure aligned on 128bits words
};
static_assert(sizeof(InstrumentConfigurationAckMessage) == 48, "Invalid InstrumentConfigurationAckMessage size");


struct ENYX_PACKED_STRUCT TickToCancelNotificationMessage {
    //16B
    struct FpgaToCpuHeader header; //version == 1, msgtype == 1, length ==
    uint64_t trade_summary_price; // price that triggered the send
    //16B
    uint64_t book_top_level_price; // book top level price
    uint64_t threshold; // algo threshold
    // 16B
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t sent_collection_id; // triggered collection id
    uint8_t is_bid; /// Whether the configuration fot this instrument is enabled or not.
    std::array<uint8_t, 9> reserved; // pad to ensure 128b
};
static_assert(sizeof(TickToCancelNotificationMessage) == 48, "Invalid TickToCancelNotificationMessage size");

struct ENYX_PACKED_STRUCT  TickToTradeNotificationMessage {
    //16B
    struct FpgaToCpuHeader header; //version == 1, msgtype == 1, length ==
    uint64_t trade_summary_price; // price that triggered the send
    //16B
    uint64_t threshold_price; // threshold for algo
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t sent_collection_id; // triggered collection id
    uint8_t is_bid; /// Whether the configuration fot this instrument is enabled or not.
    std::array<uint8_t, 1> reserved; //ensure aligned on 128bits words
};
static_assert(sizeof(TickToTradeNotificationMessage) == 32, "Invalid TickToTradeNotificationMessage size");

std::ostream&
operator<<(std::ostream&, const InstrumentConfiguration&);

std::ostream&
operator<<(std::ostream&, const InstrumentConfigurationAckMessage&);

std::ostream&
operator<<(std::ostream&, const TickToCancelNotificationMessage&);

std::ostream&
operator<<(std::ostream&, const TickToTradeNotificationMessage&);


} // demo namespace
} // hwstrat namespace

} // namespace oe
} // namespace enyx
