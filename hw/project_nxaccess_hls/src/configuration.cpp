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

#include "configuration.hpp"
#include "messages.hpp"

#include "../include/enyx/oe/hwstrat/helpers.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {


/// Print helper for cpu2fpga_header, will be moved out of there later
std::ostream& operator<<(std::ostream& os, const enyx::oe::hwstrat::cpu2fpga_header& header)
{
    os << "(version, dest, msg_type, ack_req, ts, len) = ("
       << int(header.version) << ", "
       << int(header.dest) << ", "
       << int(header.msg_type) << ", "
       << int(header.ack_request) << ", "
       << uint32_t(header.timestamp) << ", "
       << uint16_t(header.length) << ")";
    return os;
}


/// Converts software message structure to internal storage structure
void
convert(InstrumentConfiguration::instrument_configuration_data_item & internal_data,
                    const user_dma_update_instrument_configuration& msg) {
    internal_data.enabled = msg.enabled;
    internal_data.tick_to_cancel_collection_id = msg.tick_to_cancel_collection_id;
    internal_data.tick_to_cancel_threshold = msg.tick_to_cancel_threshold;
    internal_data.tick_to_trade_ask_collection_id = msg.tick_to_trade_ask_collection_id;
    internal_data.tick_to_trade_ask_price = msg.tick_to_trade_ask_price;
    internal_data.tick_to_trade_bid_collection_id = msg.tick_to_trade_bid_collection_id;
    internal_data.tick_to_trade_bid_price = msg.tick_to_trade_bid_price;

}

/// Converts data words from User DMA to software message structure
void
read_word(user_dma_update_instrument_configuration& ret, ap_uint<128> word, int word_index) {
   #pragma HLS function_instantiate variable=word_index
    switch(word_index) {
    case 1: {
        enyx::oe::hwstrat::read_word(ret.header, word(127,64));
        ret.tick_to_cancel_threshold = word(63,0);
        break;
    }
    case 2: {
        ret.tick_to_trade_bid_price = word(127,64);
        ret.tick_to_trade_ask_price = word(63,0);
        break;
    }
    case 3: {
        ret.instrument_id = word(127, 96);
        ret.tick_to_trade_bid_collection_id = word(95, 80);
        ret.tick_to_cancel_collection_id = word(79, 64);
        ret.tick_to_trade_ask_collection_id = word(63, 48);
        ret.enabled = word(47, 40);
        break;
    }
    default:
        assert(false && "Handling only 4 words for user_dma_update_instrument_configuration decoding");
    }
}

/// Converts User DMA to software message structure to data wordss
void
write_word(user_dma_update_instrument_configuration& in, ap_uint<128>& out_word, int word_index) {
   #pragma HLS function_instantiate variable=word_index
    switch(word_index) {
    case 1: {
        out_word(127, 64) = enyx::oe::hwstrat::get_word(in.header); //64
        out_word(63,0) = in.tick_to_cancel_threshold;
        break;
    }
    case 2: {
        out_word(127,64)=  in.tick_to_trade_bid_price;  // 64
        out_word(63,0) = in.tick_to_trade_ask_price; // 64
        break;
    }
    case 3: {
        out_word(127,96) = in.instrument_id; //32
        out_word(95, 80) = in.tick_to_trade_bid_collection_id; //16
        out_word(79, 64) = in.tick_to_cancel_collection_id; //16
        out_word(63, 48) = in.tick_to_trade_ask_collection_id; //16
        out_word(47, 40) = in.enabled; //8
        break;
    }
    default:
        assert(false && "Handling only 4 words for user_dma_update_instrument_configuration decoding");
    }
}

std::ostream& operator<<(std::ostream& os, const user_dma_update_instrument_configuration& conf)
{
    os << "header: " << conf.header; // TODO : remaining fields
    return os;
}

void
InstrumentConfiguration::p_handle_instrument_configuration(hls::stream<enyx::hfp::hls::dma_user_channel_data_in> & conf_in,
                                                          hls::stream<InstrumentConfiguration::read_instrument_data_request> (& req_in)[2],
                                                          hls::stream<instrument_configuration_data_item> (& req_out)[2],
                                                          hls::stream<enyx::hfp::hls::dma_user_channel_data_out> & conf_out) {

#pragma HLS INLINE recursive
#pragma HLS PIPELINE enable_flush

    static enum  { IDLE,  /// doing nothing
                   IGNORE_PACKET, /// ignore incoming packet
                   READ_CONF_WORD2, /// will process word 2 of DMA input
                   READ_CONF_WORD3 /// will process word 3 of DMA input
                 } current_state; /// current state in FSM
    #pragma HLS RESET variable=current_state

    static user_dma_update_instrument_configuration current_dma_message_read; /// DMA message being parsed message.
    static instrument_configuration_data_item write_data ;
    static InstrumentConfiguration::instrument_configuration_data_item values[InstrumentConfiguration::instrument_count];

    switch(current_state) {

    case IDLE:
    {
            if(!conf_in.empty()) {
                   enyx::hfp::hls::dma_user_channel_data_in _read = conf_in.read();
                   read_word(current_dma_message_read, _read.data, 1); // convert word 1 into struct
                   if((current_dma_message_read.header.dest == enyx::oe::nxaccess_hw_algo::InstrumentDataConfiguration)
                           && (current_dma_message_read.header.msg_type == InstrumentConfiguration::UpdateInstrumentData)
                           && (current_dma_message_read.header.version == 1))
                   {
                       std::cout << "[CONF] Incoming configuration message : accepted. ack_request="
                                 << int(current_dma_message_read.header.ack_request)  << "\n";

                       current_state = READ_CONF_WORD2; // now process second word of packet
                   } else {
                       std::cout << "[CONF] Incoming configuration message : message unknown, ignoring ! \n" ;
                       current_state = IGNORE_PACKET;
                   }

            } else {
                // process read requests from read request bus
                for(int i = 0; i != 2; ++i) {
                    if(!req_in[i].empty()) {
                        req_out[i].write(values[req_in[i].read()]);
                    }
                }

            }
            break;
    }
    case READ_CONF_WORD2: {
        if(!conf_in.empty()) {
            std::cout << "[CONF] processing word 2 of configuration message \n";
            enyx::hfp::hls::dma_user_channel_data_in _read = conf_in.read();
            read_word(current_dma_message_read, _read.data, 2); // convert word 2 into struct
            current_state = READ_CONF_WORD3;
        }
        for(int i = 0; i != 2; ++i) {
            if(!req_in[i].empty()) {
                req_out[i].write(values[req_in[i].read()]);
            }
        }
        break;
    }
    case READ_CONF_WORD3:{
        if(!conf_in.empty()) {

            std::cout << "[CONF] processing word 3 of configuration message \n";
            enyx::hfp::hls::dma_user_channel_data_in _read = conf_in.read();
            read_word(current_dma_message_read, _read.data, 3); // convert word 4 into struct
            // convert it to instrument_configuration_data_item
            convert(write_data, current_dma_message_read);
            values[current_dma_message_read.instrument_id] = write_data;

            std::cout << "[CONF] Configuration for instrument "
                      << std::hex << std::showbase << current_dma_message_read.instrument_id
                      << std::dec<< " updated."
                      << " tick_to_cancel_threshold=" << std::hex << current_dma_message_read.tick_to_cancel_threshold
                      << " tick_to_cancel_collid=" <<    std::hex << current_dma_message_read.tick_to_cancel_collection_id
                      << "\n";

            enyx::hfp::hls::dma_user_channel_data_out _output = enyx::hfp::hls::dma_user_channel_data_out();
            _output.data(63,0) = 0xcafebabe; // sends this to software, just for fun. Future version will implement an ack mechanism.
            _output.last = 1;
            conf_out.write(_output);
            current_state = IDLE;
        }
        break;
    }
    case IGNORE_PACKET: { /// goal of this step is to process an unknown packet and
                          /// let it through without parsing it
        if(!conf_in.empty()) {

            std::cout << "[CONF] Ignoring word from DMA. \n";
            enyx::hfp::hls::dma_user_channel_data_in _read = conf_in.read();
            if(_read.last == 1) { /// it's the end of the packet, let's go back in IDLE state to process requests.
                current_state = IDLE;
            }
        }
        for(int i = 0; i != 2; ++i) {
            if(!req_in[i].empty()) {
                req_out[i].write(values[req_in[i].read()]);
            }
        }
        break;
    }
    }

}

}
}
}
