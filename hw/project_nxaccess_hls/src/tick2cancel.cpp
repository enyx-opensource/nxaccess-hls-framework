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

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <cassert>
#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/md/hw/nxbus.hpp"
#include "tick2cancel.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

using namespace enyx::oe::hwstrat;

static void fill_header(user_dma_tick2cancel_notification& notification, Tick2cancel::notifications_messages_types message_type) {
    notification.header.reserved = 0;
    notification.header.error = 0;
    notification.header.version = 1;
    notification.header.source = enyx::oe::nxaccess_hw_algo::Tick2cancel;

    // we encode the side of the decision but it's only for showing that we have a message type that could be use
    // to transport several type of messages to host
    notification.header.msg_type = uint8_t(message_type);
    notification.header.length = 0x0030; // force length value since sizeof yield incorrect value because vivado (as of 2018.3) has trouble with packed structure
    // notification.header.length = sizeof(user_dma_tick2cancel_notification);
}


/**
 * @brief Tick2cancel::preprocess_nxbus Process nxbus data and performs read request to Book & Instrument managers.
 */

void Tick2cancel::preprocess_nxbus(hls::stream<nxmd::nxbus_axi> & nxbus_axi_in,
                                    hls::stream<InstrumentConfiguration::read_instrument_data_request> & instrument_data_req,
                                    hls::stream<enyx::md::hw::BooksData<2,256>::read_book_data_request> & book_req_out, hls::stream<ContextData> & decision_data_out) {
#pragma HLS INLINE recursive
#pragma HLS PIPELINE enable_flush

    // Keep a copy of the data we need to react on in the 'trigger' function
    // We used to store a whole
    // decision data
    static Tick2cancel::ContextData decision_data;

    static bool is_end_of_extra = true; // assume previous message contains end of extra
    #pragma HLS RESET variable=is_end_of_extra

    if (! nxbus_axi_in.empty()) {
        // Local variables
        nxmd::nxbus_axi nxbus_data_in;
        nxmd::nxbus nxbus_word_in;
        // Check if available to have a non blocking read
        nxbus_data_in = nxbus_axi_in.read();
        nxbus_word_in = static_cast<nxmd::nxbus>(nxbus_data_in);

        // User could do some instrument filtering for this strategy here but
        // in this Demonstration it is considered that all the feed handler is
        // configured to publish updates only on the desired instruments:
        // if (not_subscribed(nxbus_word_in.instr_id) {
        //     return ;
        // }

        if(is_end_of_extra) { // if previous burst is the end of a burst sequence

            if (nxbus_word_in.opcode == nxmd::NXBUS_OPCODE_MISC_INPUT_PKT_INFO) {

                std::cout << "[TICK2CANCEL] [nxbus timestamp " << std::hex << nxbus_word_in.timestamp << "] "
                            << "Processing : Misc Input Info message seqnum=" << nxbus_word_in.data0 << std::dec << std::endl;

                // Keep sequence number
                decision_data.sequence_number = nxbus_word_in.data0;

            } else if (nxbus_word_in.opcode == nxmd::NXBUS_OPCODE_TRADE_SUMMARY ) {

                std::cout << "[TICK2CANCEL] [nxbus timestamp " << std::hex << nxbus_word_in.timestamp << "] "
                            << "Processing : Trade Summary message price=" << nxbus_word_in.price
                            << " -> request to book memory & configuration "
                            << std::dec << std::endl;

                // prepare & transfer decision data to trigger()
                decision_data.timestamp = nxbus_word_in.timestamp;
                decision_data.price = nxbus_word_in.price;
                decision_data.instr_id = nxbus_word_in.instr_id;
                decision_data_out.write(decision_data);

                instrument_data_req.write(nxbus_word_in.instr_id); // Request the instrument's configuration
                book_req_out.write(nxbus_word_in.instr_id); // Request instrument's latest book to the book manager

            } else {
                // Here, we do nothing, as we don't know what to do
                // std::cout << "[TICK2CANCEL] [nxbus timestamp " << std::hex << nxbus_word_in.timestamp << "] "
                // << "Ignored nxBus command : opcode=" << std::hex << nxbus_word_in.opcode  << std::endl;
            }
            //book_req_out.write(nxbus_word_in.instr_id); // Request instrument's latest book to the book manager

        }
        is_end_of_extra = nxbus_word_in.end_of_extra; // keep this information in memory for next message

    } else {
        // std::cout << "nxbus input was empty" << std::endl;
    }
}

/**
 * @brief Tick2cancel::trigger Perform trigger action if algorithmic conditions are met.
 */
void Tick2cancel::trigger(hls::stream<InstrumentConfiguration::instrument_configuration_data_item> & instrument_data_resp,
                          hls::stream<enyx::md::hw::BooksData<2,256>::book_entry> & books_in,
                          hls::stream<nxoe::trigger_command_axi> & trigger_axibus_out,
                          hls::stream<user_dma_tick2cancel_notification>& tick2cancel_notification_out,
                          hls::stream<Tick2cancel::ContextData>& decision_data_in) {

#pragma HLS INLINE recursive
#pragma HLS PIPELINE enable_flush

    // Waiting for the instrument's configuration & latest books data
    if(!instrument_data_resp.empty() &&
            !books_in.empty() &&
            !decision_data_in.empty()) {
        // Read conf data & books data
        InstrumentConfiguration::instrument_configuration_data_item trigger_config = instrument_data_resp.read();
        enyx::md::hw::BooksData<2,256>::book_entry book = books_in.read();
        Tick2cancel::ContextData decision_data = decision_data_in.read();

        // Algorithm : we test whether current trade summary price is out of a "threashold(ed)-scope", and
        // if so, trigger a collection for, presumability, cancelling some orders.
        if ((book.bid_present) && trigger_config.enabled
                && (decision_data.price <= book.bid_toplevel_price - trigger_config.tick_to_cancel_threshold)
                && (trigger_config.tick_to_cancel_threshold != 0)) {

            std::cout << "[TICK2CANCEL] trade summary below buy threshold ts=" << std::hex << decision_data.timestamp << " "
                      << " price="  << decision_data.price << " <= threshold price=" << (book.bid_toplevel_price - trigger_config.tick_to_cancel_threshold)
                        << " -> triggering collection "  << std::hex << trigger_config.tick_to_cancel_collection_id << std::dec <<  std::endl;

            std::cout << "trigger collection #" << std::hex << decision_data.timestamp << "\n";

            nxoe::trigger_collection(trigger_axibus_out,
                                     trigger_config.tick_to_cancel_collection_id, // Collection to Trigger
                                     decision_data.timestamp, // Timestamp can be passed as a unique ID
                                     (uint64_t)0x1ee1312cafedeca, // Specify any 128 bit value that you want
                                     (uint8_t)1, // 1 means tick-to-cancel trigger
                                     (uint8_t)0 // 0 means trade summary below bid threshold
                                     ); // Other Arguments don't have to be specified if not needed

             // write notification in 1clk max
            user_dma_tick2cancel_notification notification;
            fill_header(notification, AlgoCancelledOnBidSide);
            //applicative layer
            notification.sent_collection_id = trigger_config.tick_to_cancel_collection_id;
            notification.trade_summary_price = decision_data.price;
            notification.book_top_level_price = book.bid_toplevel_price;
            notification.instrument_id = decision_data.instr_id;
            notification.threshold = trigger_config.tick_to_cancel_threshold;
            notification.is_bid = 1;

            tick2cancel_notification_out.write(notification); // write to the internal notification data bus

        } else if ((book.ask_present) && trigger_config.enabled
                   && (decision_data.price >= book.ask_toplevel_price + trigger_config.tick_to_cancel_threshold)
                   && (trigger_config.tick_to_cancel_threshold != 0)) {

            std::cout << "[TICK2CANCEL] trade summary above ask threshold ts=" << std::hex << decision_data.timestamp << " "
                      << " price="  << decision_data.price << " >= threshold price=" << (book.bid_toplevel_price + trigger_config.tick_to_cancel_threshold)
                        << " -> triggering collection "  << std::hex << trigger_config.tick_to_cancel_collection_id << std::dec <<  std::endl;
            std::cout << "trigger collection #" << std::hex << decision_data.timestamp << "\n";
            nxoe::trigger_collection(trigger_axibus_out,
                                     trigger_config.tick_to_cancel_collection_id, // Collection to Trigger
                                     decision_data.timestamp, // Timestamp can be passed as a unique ID
                                     (uint64_t)0x1ee1314cafedeca, // Specify any 128 bit value that you want
                                     (uint8_t)1, // 1 means tick-to-cancel trigger
                                     (uint8_t)1 // 1 means trade summary above ask threshold
                                     ); // Other Arguments don't have to be specified if not needed

            // write notification in 1clk max
            user_dma_tick2cancel_notification notification;
            fill_header(notification, AlgoCancelledOnAskSide);
            //applicative layer
            notification.sent_collection_id = trigger_config.tick_to_cancel_collection_id;
            notification.trade_summary_price = decision_data.price;
            notification.book_top_level_price = book.ask_toplevel_price;
            notification.instrument_id = decision_data.instr_id;
            notification.threshold = trigger_config.tick_to_cancel_threshold;
            notification.is_bid = 0;
            tick2cancel_notification_out.write(notification);

        }
    }

}

enyx::hfp::dma_user_channel_data_out
Tick2cancel::notification_to_word(const user_dma_tick2cancel_notification& notif_in, int word_index)
{
    enyx::hfp::dma_user_channel_data_out out_word;

    switch(word_index) {
        case 1: {
        out_word.data(127, 64) =  enyx::oe::hwstrat::get_word(notif_in.header); //64
        out_word.data(63,0) = notif_in.trade_summary_price; // 64
        out_word.last = 0;
        break;
        }
        case 2: {
            out_word.data(127,64)=  notif_in.book_top_level_price;  // 64
            out_word.data(63,0) = notif_in.threshold; // 64
            out_word.last = 0;
            break;
        }
        case 3: {
            out_word.data(127,96) = notif_in.instrument_id; //32
            out_word.data(95, 80) = notif_in.sent_collection_id; //16
            out_word.data(79, 72) = notif_in.is_bid; //8
            out_word.data(72-1, 0) = 0;
            out_word.last = 1; // last packet of the word sequence
            break;
        }
         default:
            assert(false && "Handling only 3 words for user_dma_tick2cancel_notification encoding");

    }
    return out_word;
}

}}}
