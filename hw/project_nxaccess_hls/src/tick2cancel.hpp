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

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/md/hw/nxbus.hpp"
#include "../include/enyx/md/hw/books.hpp"
#include "configuration.hpp"

namespace nxmd = enyx::md::hw;
namespace nxoe  = enyx::oe::hwstrat;

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

/**
 * @brief The Tick2cancel strategy. This strategy is implemented with a 2-process approach
 * that ensure it has not bandwith problem and can handle one nxbus word at each clock cycle.
 */
class Tick2cancel {
public:

    /**
     * @brief Data required for taking a decision, extracted from the market message.
     */
    struct ContextData {
        ap_uint<64>  price;                 // price that triggered
        ap_uint<64>  timestamp;             // timestamp of the triggering event
        ap_uint<64>  sequence_number;       // sequence number of the market packet
        ap_uint<16>  source_id;             // multicast source id of the market packet
        ap_uint<24>  instr_id;              // instrument id
    };

    enum notifications_messages_types {
        AlgoCancelledOnAskSide = 1, // When decision is taken for ask side
        AlgoCancelledOnBidSide = 2, // When decision is taken for bid side
    };

    /// Tick 2 Cancel strategy

    /**
     * @brief Tick2cancel::preprocess_nxbus Process nxbus data and performs read request to Book & Instrument managers.
     */
    static void
    preprocess_nxbus( hls::stream<nxmd::nxbus_axi> & nxbus_axi_in,
                        hls::stream<InstrumentConfiguration::read_instrument_data_request> & instrument_data_req,
                        hls::stream<enyx::md::hw::BooksData<2,256>::read_book_data_request> & book_req_out,
                      hls::stream<ContextData> &decision_data_out);

    /**
     * @brief Tick2cancel::trigger Perform trigger action if algorithmic conditions are met.
     */
    static void
    trigger(hls::stream<InstrumentConfiguration::instrument_configuration_data_item> & instrument_data_in,
              hls::stream<enyx::md::hw::BooksData<2,256>::book_entry> & books_in,
              hls::stream<nxoe::trigger_command_axi> & trigger_axibus_out,
              hls::stream<user_dma_tick2cancel_notification>& tick2cancel_notification_out,
            hls::stream<ContextData> &decision_data_in);


    static enyx::hfp::dma_user_channel_data_out
    notification_to_word(const user_dma_tick2cancel_notification& notif_in, int word_index);
}; // class
}}} // Namespaces
