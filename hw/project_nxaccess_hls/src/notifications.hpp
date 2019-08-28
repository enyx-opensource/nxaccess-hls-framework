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
#include "configuration.hpp" // for enyx::hfp::hls, FIXME.

namespace nxmd = enyx::md::hw;
namespace nxoe  = enyx::oe::hwstrat;

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

// using namespace enyx::hfp::hls;
// void write_word(user_dma_update_instrument_configuration& in, ap_uint<128>& out_word, int word_index);


/// Handles notifications from modules, and broadcast them into the DMA 
/// This process allows some simplification in other modules
class Notifications
{
  public:

    static void
    p_broadcast_notifications(hls::stream<user_dma_tick2cancel_notification> &tick2cancel_in, 
                              hls::stream<user_dma_tick2trade_notification> &tick2trade_in,
                              hls::stream<user_dma_update_instrument_configuration_ack> &config_acks_in,
                              hls::stream<enyx::hfp::hls::dma_user_channel_data_out> & conf_out);

  
};

}
}
}
