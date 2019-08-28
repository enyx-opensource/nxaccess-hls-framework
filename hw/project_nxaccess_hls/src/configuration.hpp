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

#pragma once

#include <cstddef>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>

#include "../include/enyx/md/hw/nxbus.hpp"

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/oe/hwstrat/msg_headers.hpp"
#include "../include/enyx/oe/hwstrat/helpers.hpp"


#include "messages.hpp"
#include "../include/enyx/hfp/hls/hfp.hpp"

namespace nxmd = enyx::md::hw;
namespace nxoe  = enyx::oe::hwstrat;

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

using namespace enyx::hfp::hls;


class InstrumentConfiguration
{
  public:
    typedef uint32_t read_instrument_data_request ; /// read instrument data request in memory
    static std::size_t const instrument_count = 256;

    static enum {
        UpdateInstrumentData = 1, // Update Instrument data
    } messages_types;

    /// memory structure used for storing instrument configuration
    /// layout of the packet sent by software to FPGA
    /// see messages.hpp for message layout reference
    /// This is message_type = UpdateInstrumentData
    struct instrument_configuration_data_item {
        ap_uint<1> enabled; // is this configuration enabled

        ap_uint<nxmd::nxbus_meta_sizes::NXBUS_SIZE_PRICE> tick_to_cancel_threshold;
        ap_uint<nxoe::trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> tick_to_cancel_collection_id;

        ap_uint<nxmd::nxbus_meta_sizes::NXBUS_SIZE_PRICE> tick_to_trade_bid_price;
        ap_uint<nxoe::trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> tick_to_trade_bid_collection_id;

        ap_uint<nxmd::nxbus_meta_sizes::NXBUS_SIZE_PRICE> tick_to_trade_ask_price;
        ap_uint<nxoe::trigger_meta_size::TRIGGER_SIZE_COLLECTION_ID> tick_to_trade_ask_collection_id;
    }; // 241 bits wide


    InstrumentConfiguration() {}

    /// Read messages from DMA and store configuration into memory, provide feedback message to DMA
    /// and answers read request to decision blocks
    static void
    p_handle_instrument_configuration(hls::stream<enyx::hfp::hls::dma_user_channel_data_in> & conf_in,
                                               hls::stream<read_instrument_data_request> (& req_in)[2],
                                               hls::stream<instrument_configuration_data_item> (& req_out)[2],
                                               hls::stream<user_dma_update_instrument_configuration_ack> & conf_out);


    static void write_word(const user_dma_update_instrument_configuration& in, enyx::hfp::hls::dma_user_channel_data_out& word,  int word_index);
    static void write_word(const user_dma_update_instrument_configuration_ack& in, enyx::hfp::hls::dma_user_channel_data_out& word, int word_index);
    
    static void read_word(user_dma_update_instrument_configuration& ret, const enyx::hfp::hls::dma_user_channel_data_in& word, int word_index);
    static void read_word(user_dma_update_instrument_configuration_ack& ret, const enyx::hfp::hls::dma_user_channel_data_in& word,  int word_index);


};

}
}
}
