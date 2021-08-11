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

enum class ModulesIds : uint8_t {
    InstrumentDataConfiguration = 8, // Module that handle instrument configuration, see configuration.hpp
    SoftwareTrigger = 9, // Not implemented yet, reserved for module handling trigger from software. // Not present in demonstration
    TickToCancel = 10,   // tick2cancel strategy
    TickToTrade = 11 // tick2trade strategy
};

/// CPU To FPGA header
struct ENYX_PACKED_STRUCT CpuToFpgaHeader {
    // first byte
    ModulesIds dest:4;      /// source fpga module id
    uint8_t version:4;     /// message format version
    // second byte
    uint8_t reserved:3;       ///
    uint8_t ack_request:1; /// Ack message request (1 = want ack)
    uint8_t msg_type:4;    /// message_type, defined per module

    uint32_t timestamp;   /// future use : hw timestamp, unused by soft
    uint16_t length;    /// message format version
};
static_assert(sizeof(CpuToFpgaHeader) == 8, "Invalid CpuToFpgaHeader size");

struct ENYX_PACKED_STRUCT FpgaToCpuHeader {
    // first byte
    uint8_t source:4;  /// source fpga module id
    uint8_t version:4;     /// message format version
    // second byte
    uint8_t reserved:3;     /// ack
    uint8_t error:1;     /// error bit in case message is not processed correctly
    uint8_t msg_type:4;  /// message_type

    uint32_t timestamp;   /// future use : hw timestamp, unused for now.
    uint16_t length;    /// message format version
};
static_assert(sizeof(FpgaToCpuHeader) == 8, "Invalid FpgaToCpuHeader size");


/**
 * @brief Configuration of an instrument.
 */
struct ENYX_PACKED_STRUCT InstrumentConfiguration {
    uint64_t tick_to_cancel_threshold; // price for tick 2 cancel
    uint64_t tick_to_trade_bid_price; // price for tick 2 cancel
    uint64_t tick_to_trade_ask_price; // price for tick 2 cancel
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t tick_to_trade_bid_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_cancel_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_trade_ask_collection_id; // collection id to trigger if price under threshold
    uint8_t enabled; /// Whether the configuration fot this instrument is enabled or not.
};


struct ENYX_PACKED_STRUCT InstrumentConfigurationMessage {
    struct CpuToFpgaHeader header; // 64 bits
    InstrumentConfiguration configuration;
    char pad2[5]; //ensure aligned on 128bits words
};
static_assert(sizeof(InstrumentConfigurationMessage) == 48, "Invalid InstrumentConfigurationMessage size");

/**
 * @brief Software trigger specific header.
 */
struct ENYX_PACKED_STRUCT TriggerWithArgsHeader {
    uint16_t               collectionId {0}; /// Collection Id
    uint8_t                argBitmap {0};    /// The arguments that will be present in this trigger request. To be mapped directly on the arg_valid bus.
    std::array<uint8_t, 5> reserved {};      /// Padding
};
static_assert(sizeof(TriggerWithArgsHeader) == 8, "Invalid TriggerWithArgsHeader size");

/**
 * @brief  Arguments of the collection to trigger.
 */
struct ENYX_PACKED_STRUCT TriggerArgs {
    std::array<uint8_t, 16> arg0 {};  /// First argument  - Mandatory
    std::array<uint8_t, 16> arg1 {};  /// Second argument - Optional
    std::array<uint8_t, 16> arg2 {};  /// Third argument  - Optional
    std::array<uint8_t, 16> arg3 {};  /// Forth argument  - Optional
    std::array<uint8_t, 16> arg4 {};  /// Fifth argument  - Optional
};
static_assert(sizeof(TriggerArgs) == 80, "Invalid TriggerWithArgsuint size");

/**
 * @brief Message to send to trigger a collection with args.
 *        The size of the message will vary depending on the arg_bitmap.
 */
struct ENYX_PACKED_STRUCT TriggerWithArgsMessage {
    CpuToFpgaHeader       header;
    TriggerWithArgsHeader trigger;
    TriggerArgs           args;
};
static_assert(sizeof(TriggerWithArgsMessage) == 96, "Invalid TriggerWithArgsMessage size");


struct ENYX_PACKED_STRUCT InstrumentConfigurationAckMessage {
    //16B
    struct FpgaToCpuHeader header; //version == 1, msgtype == 1, length ==
    uint64_t tick_to_cancel_threshold; // price for tick 2 cancel
    //16B
    uint64_t tick_to_trade_bid_price; // price for tick 2 cancel
    uint64_t tick_to_trade_ask_price; // price for tick 2 cancel
    //16B
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t tick_to_trade_bid_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_cancel_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_trade_ask_collection_id; // collection id to trigger if price under threshold
    uint8_t enabled; /// Whether the configuration for this instrument is enabled or not.
    char padding[5]; //ensure aligned on 128bits words
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
    char padding[9]; // pad to ensure 128b
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
    char padding[1]; //ensure aligned on 128bits words
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
