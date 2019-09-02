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


/// Complete message layout to configure an instrument trigger, for CPU->FPGA comm
#ifdef ENYX_NO_HLS_SUPPORT // do not use #pragma pack() with Vivado
    #pragma pack(1)
#endif
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

#  if __GNUC_MAJOR__ >= 5  // introduced with C++11 standard
  static_assert(64 == sizeof(user_dma_update_instrument_configuration), "Size of user_dma_update_instrument_configuration is invalid");
# else
   # if __GNUC_MAJOR__ >= 4 // only available on GCC 4.6+. What about clang ?
     _Static_assert(64 == sizeof(user_dma_update_instrument_configuration), "Size of user_dma_update_instrument_configuration is invalid");
   # endif
# endif


/// Complete message layout to configure an instrument trigger, for FPGA->CPU comm
#ifdef ENYX_NO_HLS_SUPPORT // do not use #pragma pack() with Vivado
    #pragma pack(1)
#endif
struct user_dma_update_instrument_configuration_ack {
    //16B
    struct enyx::oe::hwstrat::fpga2cpu_header header; //version == 1, msgtype == 1, length ==
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
#  if __GNUC_MAJOR__ >= 5  // introduced with C++11 standard
  static_assert(64 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
# else
   # if __GNUC_MAJOR__ >= 4 // only available on GCC 4.6+. What about clang ?
     _Static_assert(64 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
   # endif
# endif



/// Complete message layout to configure an instrument trigger, for FPGA->CPU comm
#ifdef ENYX_NO_HLS_SUPPORT // do not use #pragma pack() with Vivado
    #pragma pack(1)
#endif
struct user_dma_tick2cancel_notification {
    //16B 
    struct enyx::oe::hwstrat::fpga2cpu_header header; //version == 1, msgtype == 1, length ==
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
#  if __GNUC_MAJOR__ >= 5  // introduced with C++11 standard
  static_assert(48 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
# else
   # if __GNUC_MAJOR__ >= 4 // only available on GCC 4.6+. What about clang ?
     _Static_assert(48 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
   # endif
# endif


/// Complete message layout to configure an instrument trigger, for CPU2FPGA comm
#ifdef ENYX_NO_HLS_SUPPORT // do not use #pragma pack() with Vivado
    #pragma pack(1)
#endif
struct user_dma_tick2trade_notification {
    //16B
    struct enyx::oe::hwstrat::fpga2cpu_header header; //version == 1, msgtype == 1, length ==
    uint64_t trade_summary_price; // price that triggered the send
    //16B
    uint64_t threshold_price; // threshold for algo
    uint32_t instrument_id; /// instrument id to trigger on
    uint16_t sent_collection_id; // triggered collection id  
    uint8_t is_bid; /// Whether the configuration fot this instrument is enabled or not.
    char padding[1]; //ensure aligned on 128bits words

};
#  if __GNUC_MAJOR__ >= 5  // introduced with C++11 standard
  static_assert(32 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
# else
   # if __GNUC_MAJOR__ >= 4 // only available on GCC 4.6+. What about clang ?
     _Static_assert(32 == sizeof(user_dma_update_instrument_configuration_ack), "Size of user_dma_update_instrument_configuration is invalid");
   # endif
# endif


// Modules Ids for this architecture
enum fpga_modules_ids {
    Reserved0, // Reserved for enyx
    Reserved1, // Reserved for enyx
    Reserved2, // Reserved for enyx
    Reserved3, // Reserved for enyx
    Reserved4, // Reserved for enyx
    Reserved5, // Reserved for enyx
    Reserved6, // Reserved for enyx
    Reserved7, // Reserved for enyx
    InstrumentDataConfiguration = 8, // Module that handle instrument configuration, see configuration.hpp
    SoftwareTrigger = 9, // Not implemented yet, reserved for module handling trigger from software. // Not present in demonstration
    Tick2cancel = 10,   // tick2cancel strategy
    Tick2trade = 11 // tick2trade strategy
}; // application specific definition of module ids.

}
}
}

#include "../include/enyx/hfp/hfp.hpp"


#endif // MESSAGES_HPP
