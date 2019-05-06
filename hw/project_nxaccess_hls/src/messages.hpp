//--------------------------------------------------------------------------------
//--! Enyx Confidential
//--!
//--! Organization:          Enyx
//--! Project Identifier:    010 - Enyx nxAccess HLS Framework
//--! Author:                Raphael Charolois (raphael.charolois@enyx.com)
//--!
//--! © Copyright            Enyx 2019
//--! © Copyright Notice:    The source code for this program is not published or otherwise divested of its trade secrets, 
//--!                        irrespective of what has been deposited with the U.S. Copyright Office.
//--------------------------------------------------------------------------------

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

/// This file defines the FPGA/CPU messages layouts exchanges in the nxAccess HLS demo project.
/// This file can be reused by software only lib, no dependency on HLS libs.

#include "../include/enyx/oe/hwstrat/msg_headers.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {


/// Complete message layout to configure an instrument trigger, for CPU2FPGA comm
#pragma pack(1)
struct user_dma_update_instrument_configuration {
    // word 1
    struct enyx::oe::hwstrat::cpu2fpga_header header; // 64 bits
    uint64_t tick_to_cancel_threshold; // price for tick 2 cancel
    // word 2
    uint64_t tick_to_trade_bid_price; // price for tick 2 cancel
    uint64_t tick_to_trade_ask_price; // price for tick 2 cancel
    // word 3
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t tick_to_trade_bid_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_cancel_collection_id; // collection id to trigger if price under threshold
    uint16_t tick_to_trade_ask_collection_id; // collection id to trigger if price under threshold
    uint8_t enabled; /// Whether the configuration fot this instrument is enabled or not.
    char pad2[5]; //ensure aligned on 128bits words

};

// only on GCC 4.6
//_Static_assert(64 == sizeof(user_dma_update_instrument_configuration), "Size of user_dma_update_instrument_configuration is invalid");
//static_assert(64 == sizeof(user_dma_update_instrument_configuration), "Size of user_dma_update_instrument_configuration is invalid");

/// Complete message layout to configure an instrument trigger, for CPU2FPGA comm
#pragma pack(1)
struct user_dma_update_instrument_configuration_ack {
    struct enyx::oe::hwstrat::fpga2cpu_header header; //version == 1, msgtype == 1, length ==
    uint8_t enabled; /// Whether the configuration fot this instrument is enabled or not.
    uint32_t instrument_id; /// instrument id to trigger on

    uint64_t tick_to_cancel_threshold; // price for tick 2 cancel
    uint16_t tick_to_cancel_collection_id; // collection id to trigger if price under threshold

    uint64_t tick_to_trade_bid_price; // price for tick 2 cancel
    uint16_t tick_to_trade_bid_collection_id; // collection id to trigger if price under threshold

    uint64_t tick_to_trade_ask_price; // price for tick 2 cancel
    uint16_t tick_to_trade_ask_collection_id; // collection id to trigger if price under threshold
    char padding[21]; //ensure aligned on 128bits words

};
#  if __GNUC_MAJOR__ >= 5  // introduced with C++11 standard
  static_assert(64 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
# else
   # if __GNUC_MAJOR__ >= 4 // only available on GCC 4.6+. What about clang ?
     _Static_assert(64 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
   # endif
# endif
// Modules Ids for this architecture
enum {
    Reserved0, // Reserved for enyx
    Reserved1, // Reserved for enyx
    Reserved2, // Reserved for enyx
    Reserved3, // Reserved for enyx
    Reserved4, // Reserved for enyx
    Reserved5, // Reserved for enyx
    Reserved6, // Reserved for enyx
    Reserved7, // Reserved for enyx
    InstrumentDataConfiguration = 8, // Module that handle instrument configuration, see configuration.hpp
    SoftwareTrigger = 9, // Not implemented yet, reserved for module handling trigger from software.
} fpga_modules_ids;



}
}
}

#endif // MESSAGES_HPP
