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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H


#include <cstddef>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>
#include <cassert>

#include "../include/enyx/md/hw/nxbus.hpp"

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/oe/hwstrat/msg_headers.hpp"
#include "../include/enyx/oe/hwstrat/helpers.hpp"


#include "messages.hpp"
#include "../include/enyx/hfp/hfp.hpp"
#include "notifications.hpp"

/// dependency upon all producing modules. See comments below for this design choice (low latency driven)
#include "configuration.hpp"
#include "tick2cancel.hpp"
#include "tick2trade.hpp"
#include "tcp_consumer.hpp"


namespace nxmd = enyx::md::hw;
namespace nxoe  = enyx::oe::hwstrat;

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

/// Converts specific module data buses to DMA "raw 128b data bus".
/// A better design would be to be agnostic and only consider 128bits "data" buses as inputs of this process.
/// However, it means that each module has to handle the FSM that converts data to a 128 bits data bus. Also,
/// this adds latency into the core components.
/// We can consider this not clean from a design & maintenance point of view ; right. Another approach might 
/// be to use variadic templates providing writers functions so that this code could be generic. Its complexity
/// would not serve anymore the documentation purpose.
void Notifications::p_broadcast_notifications(
    hls::stream<user_dma_tick2cancel_notification> &tick2cancel_in, 
    hls::stream<user_dma_tick2trade_notification> &tick2trade_in,
    hls::stream<user_dma_update_instrument_configuration_ack> &config_acks_in,
    hls::stream<user_dma_tcp_consumer_notification> &tcp_consumer_in,
    
    hls::stream<enyx::hfp::dma_user_channel_data_out> & conf_out)
{
#pragma HLS INLINE recursive
#pragma HLS PIPELINE enable_flush

    static enum  { IDLE,  /// doing nothing, waiting for input
                   WORD1, /// will write word 1 of notification
                   WORD2, /// will write word 2 of notification
                   WORD3, /// will write word 3 of notification
                //    WORD4 /// will write word 4 of notification
                 } current_state; /// current state of FSM
    #pragma HLS RESET variable=current_state
    

    static enum { Input_Tick2trade = 1,
                 Input_Tick2cancel = 2,
                 Input_Configuration = 3,
                 Input_TcpConsumer = 4, }
                 input_type;  // input type being processed
    #pragma HLS RESET variable=input_type
   
    static user_dma_tick2trade_notification             notif_t2trade; 
    static user_dma_update_instrument_configuration_ack notif_config;
    static user_dma_tick2cancel_notification            notif_t2cancel;
    static user_dma_tcp_consumer_notification           notif_tcp;

// note on this FSM : we could remove one state and spare 1 clk cycle; 
// we choose to separate the IDLE state from WORD1 for clarity.
switch(current_state) {
    // doing nothing, waiting for input 
    case IDLE  : { 
            // got an input from configuration core
            if(!config_acks_in.empty()) { 
                input_type = Input_Configuration;
                notif_config = config_acks_in.read();
                current_state = WORD1;
            }  else if (!tick2trade_in.empty()) {  
                input_type = Input_Tick2trade;
                notif_t2trade = tick2trade_in.read();
                current_state = WORD1;
            } else if (!tick2cancel_in.empty()) {
                input_type = Input_Tick2cancel;
                notif_t2cancel = tick2cancel_in.read();
                current_state = WORD1;
            } else if (!tcp_consumer_in.empty()) {
                input_type = Input_TcpConsumer;
                notif_tcp = tcp_consumer_in.read();
                current_state = WORD1;
            }   
            // else { // no status change, nothing read ! }
        break;
    } // case idle

    // will write word1
    case WORD1 : {
        switch(input_type) {

        case Input_Configuration: {
            enyx::hfp::dma_user_channel_data_out out;
            enyx::oe::nxaccess_hw_algo::InstrumentConfiguration::write_word(notif_config, out, 1);
            conf_out.write(out);
            break;
        }
        case Input_Tick2cancel: {
            enyx::hfp::dma_user_channel_data_out out;
            out = Tick2cancel::notification_to_word(notif_t2cancel, 1);
            conf_out.write(out);
            break;
        }
        case Input_Tick2trade: {
            enyx::hfp::dma_user_channel_data_out out;
            out = Tick2trade::notification_to_word(notif_t2trade, 1);
            conf_out.write(out);
            break;
        }
        case Input_TcpConsumer: {
            enyx::hfp::dma_user_channel_data_out out;
            out = TcpConsumer::notification_to_word(notif_tcp, 1);
            conf_out.write(out);
            break;
        }

        } // switch input_type
        current_state = WORD2;
        break; 
    } // case word1
    // will output second word 
    case WORD2: {
        switch(input_type) {

        case Input_Configuration: {
            enyx::hfp::dma_user_channel_data_out out;
            InstrumentConfiguration::write_word(notif_config, out, 2);
            conf_out.write(out);
            current_state = WORD3;
            break;
        }
        case Input_Tick2cancel: {
            enyx::hfp::dma_user_channel_data_out out;
            out = Tick2cancel::notification_to_word(notif_t2cancel, 2);
            conf_out.write(out);
            current_state = WORD3;
            break;
        }
        case Input_Tick2trade: {
            enyx::hfp::dma_user_channel_data_out out;
            out = Tick2trade::notification_to_word(notif_t2trade, 2);
            conf_out.write(out);
            current_state = IDLE; // we have finished for this notification type
            break;
        }
        case Input_TcpConsumer: {
            enyx::hfp::dma_user_channel_data_out out;
            out = TcpConsumer::notification_to_word(notif_tcp, 2);
            conf_out.write(out);
            current_state = IDLE; // we have finished for this notification type
            break;
        }

        } // switch input_type
        break;
    }// case word2 

     // will output second word 
    case WORD3: {
        switch(input_type) {

        case Input_Configuration: {
            enyx::hfp::dma_user_channel_data_out out;
            InstrumentConfiguration::write_word(notif_config, out, 3);
            conf_out.write(out);
            current_state = IDLE; // we have finished
            break;
        }
        case Input_Tick2cancel: {
            enyx::hfp::dma_user_channel_data_out out;
            out = Tick2cancel::notification_to_word(notif_t2cancel, 3);
            conf_out.write(out);
            current_state = IDLE;
            break;
        }
        default:
            assert(false && "Only handling 2 input types in WORD3 state ");
  
        } // switch input_type
        break;
    }// case word3
    default:
        assert(false && "invalid state in FSM ");
  
    } // switch current state 

} // process


}
}
}
#endif
