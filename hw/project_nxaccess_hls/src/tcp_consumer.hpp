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
#include "../include/enyx/md/hw/books.hpp"
#include "../include/enyx/oe/hwstrat/tcp.hpp"
#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "messages.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

/**
 * @brief TcpConsumer is the process that consumes the TCP feedback data, coming from the TCP sessions opened on the market.
 */
class TcpConsumer {
public:

    /// Consumes TCP data. Does not provide any feedback to the main algorithm.
    static void
    p_consume_tcp(
        hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & tcp_replies_in,
        hls::stream<user_dma_tcp_consumer_notification> &tick2trade_notification_out,
        hls::stream<enyx::oe::hwstrat::trigger_command_axi> & output);

    static enyx::hfp::dma_user_channel_data_out
    notification_to_word(const user_dma_tcp_consumer_notification& notif_in, int word_index);

}; // class
}}} // Namespaces
