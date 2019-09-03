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
#include "messages.hpp"

namespace nxmd = enyx::md::hw;
namespace nxoe  = enyx::oe::hwstrat;

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

/**
 * @brief The Tick2trade strategy. This strategy is implemented with a 1-process approach.
 * This implementation does not ensure that one nxbus word can be consumed at each clock cycle.
 * Theorically, it could create backpressure on the feedhandler.
 */
class Tick2trade {
public:

    enum notifications_messages_types {
        AlgoTriggeredOnAsk = 1, // When decision is taken for ask side
        AlgoTriggeredOnBid = 2, // When decision is taken for bid side
    };
    
    /// tick 2 trade strategy
    static void
    p_algo(hls::stream<nxmd::nxbus_axi> & nxbus_in,
                 hls::stream<InstrumentConfiguration::read_instrument_data_request> & instrument_data_req,
                 hls::stream<InstrumentConfiguration::instrument_configuration_data_item> & instrument_data_resp,
                 hls::stream<nxoe::trigger_command_axi> & trigger_bus_out,
                 hls::stream<user_dma_tick2trade_notification>& tick2trade_notification_out,
                 hls::stream<enyx::md::hw::BooksData<2,256>::read_book_data_request> & book_req_out,
                 hls::stream<enyx::md::hw::BooksData<2,256>::book_entry> & books_in);

    static enyx::hfp::dma_user_channel_data_out
    notification_to_word(const user_dma_tick2trade_notification& notif_in, int word_index);
}; // class
}}} // Namespaces
