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

#include <cassert>
#include <iostream>

#include "../include/enyx/oe/hwstrat/helpers.hpp"

#include "tcp_consumer.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

/**
 * @brief Fill DMA header
 */

static void
fill_header(user_dma_tcp_consumer_notification& notification) {
    notification.header.error = 0;
    notification.header.version = 1;
    notification.header.source = enyx::oe::nxaccess_hw_algo::TcpConsumer;
    notification.header.msg_type = 0;
    notification.header.length = 16 * 1;
}

uint32_t
bitCount (uint32_t value) {
    uint32_t count = 0;
    while (value > 0) {
        if ((value & 1) == 1)
            count++;
        value >>= 1;
    }
    return count;
}

void
TcpConsumer::p_consume_tcp(
    hls::stream<enyx::oe::hwstrat::tcp_reply_payload> &tcp_replies_in,
    hls::stream<user_dma_tcp_consumer_notification> &tcp_consumer_notification_out,
    hls::stream<enyx::oe::hwstrat::trigger_command_axi> &output) {
    static uint32_t bytes;
    static uint32_t words;
    static ap_uint<8> session;

    bool tcp_reply_word_available;
    tcp_reply_word_available = !tcp_replies_in.empty();

    if (! tcp_reply_word_available)
        return;

    enyx::oe::hwstrat::tcp_reply_payload tcp_reply_word;
    tcp_reply_word = tcp_replies_in.read();


    bool start_of_packet = (0 == words);
    if (start_of_packet) {
        session = tcp_reply_word.id;
    }

    bool end_of_packet = tcp_reply_word.last;
    if (end_of_packet) {
        bytes += bitCount(tcp_reply_word.keep);

        std::cout << "[tcp_comsumer] " << std::dec
            << "id: " << tcp_reply_word.id << ", "
            << "user: " << tcp_reply_word.user << ", "
            << "data: " << std::hex << tcp_reply_word.data << std::dec << ", "
            << "bytes: " << bytes << ", "
            << "words: " << words << ", "
            << std::endl;

        bool checksum_error = tcp_reply_word.user(0, 0);
        if (! checksum_error) {
            if (64 == session) {

                // note : we use tcp_data somewhere because otherwise Xilinx Vivado 2019.1 generates invalid VHDL & verilog code !!!
                // some input data must be used in the output data
                // a way to disable this code is to check for a TCP session id (tcp_reply_word.user) which is greater than 32
                // This is being reported to Xilinx.

               std::cout << "[TCP_CONSUMER] Received data for session id #2 ! Triggering collection #1024" << std::endl;

                // -- Example action code : please remove this in production unless you
                // really want it !
                // as example, we trigger collection #1

                enyx::oe::hwstrat::trigger_collection(output, 1024 + tcp_reply_word.user(7,0)+tcp_reply_word.data(7,0));
            }
        }

        if (96 == session) {

            user_dma_tcp_consumer_notification notification;
            fill_header(notification);
            notification.words = words;
            notification.bytes = bytes;
            notification.keep = tcp_reply_word.keep;
            notification.user = tcp_reply_word.user;
            notification.session = session; 
            tcp_consumer_notification_out.write(notification); // write to the internal notification data bus
        }
        
        words = 0;
        bytes = 0;
    } else {
        words++;
        bytes += enyx::oe::hwstrat::tcp_reply_payload::data_width / 8;
    }
}

enyx::hfp::dma_user_channel_data_out
TcpConsumer::notification_to_word(const user_dma_tcp_consumer_notification& notif_in, int word_index)
{
    enyx::hfp::dma_user_channel_data_out out_word;
    
    switch(word_index) {
        case 1: {
            out_word.data(127, 64) =  enyx::oe::hwstrat::get_word(notif_in.header); //64
            out_word.data(64-1, 32) = notif_in.words;
            out_word.data(32-1, 0) = notif_in.bytes;
            out_word.last = 0;
            break;
        }
        case 2: {
            out_word.data(128-1, 102)=  notif_in.keep;
            out_word.data(102-1, 96) = notif_in.user;
            out_word.data(96-1, 80) = notif_in.session; 
            out_word.last = 1;
            break;
        }
        default:
            assert(false && "Handling only 2 words for user_dma_tcp_consumer_notification encoding");
  
    }
    return out_word;
}

}}}
