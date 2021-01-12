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

// Enyx HLS framework
#include "../include/enyx/hls/arbiter.hpp"
#include "../include/enyx/hls/demuxer.hpp"
#include "../include/enyx/md/hw/books.hpp"
#include "../include/enyx/oe/hwstrat/helpers.hpp"


// nxAccess HLS algo demo
#include "top.hpp"
#include "configuration.hpp"
#include "tick2cancel.hpp"
#include "tick2trade.hpp"
#include "notifications.hpp"
#include "tcp_consumer.hpp"

#include "messages.hpp"

using namespace enyx::oe::hwstrat; // use nxAccess HLS framework.

namespace algo = enyx::oe::nxaccess_hw_algo; // use nxAccess basic example
namespace nxmd = enyx::md::hw;
namespace nxoe = enyx::oe::hwstrat;

// Modification of these constant will change the whole core behavior
// the strategy can only handle instrument_count instruments at present
static const std::size_t instrument_count = 256;
// number of trading strategies
static const std::size_t strategy_count = 2;

/// Bus indexes of the algorithms strategies
enum DecisionBusIndex {
    Tick2Cancel = 0,
    Tick2Trade = 1
};

void
algorithm_entrypoint(hls::stream<enyx::md::hw::nxbus_axi> & nxbus_in,
                     hls::stream<enyx::hfp::dma_user_channel_data_in>& user_dma_channel_data_in,
                     hls::stream<enyx::hfp::dma_user_channel_data_out>& user_dma_channel_data_out,
                     hls::stream<enyx::oe::hwstrat::trigger_command_axi> & trigger_bus_out,
                     hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & tcp_replies_in)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
// #pragma HLS STREAM variable=nxbus_in depth=1024 dim=1

#pragma HLS INTERFACE axis port=trigger_bus_out
#pragma HLS INTERFACE axis port=nxbus_in
#pragma HLS INTERFACE axis port=user_dma_channel_data_in
#pragma HLS INTERFACE axis port=user_dma_channel_data_out
#pragma HLS INTERFACE axis port=tcp_replies_in

//#pragma HLS INTERFACE register port=counter_tcpreplies_rx
#pragma HLS DATAFLOW

   // Instrument Based Configuration Access Buses

   static hls::stream<algo::InstrumentConfiguration::read_instrument_data_request> instrument_read_bus[strategy_count]; //instrument read request bus
#pragma HLS STREAM variable=instrument_read_bus depth=1

   static hls::stream<algo::InstrumentConfiguration::instrument_configuration_data_item> instrument_read_responses[strategy_count]; //instrument response bus
#pragma HLS STREAM variable=instrument_read_responses depth=1

   // Input Market Data Distribution to the various functions
   static hls::stream<nxmd::nxbus_axi> nxbus_outputs[strategy_count+1]; // demuxed/duplicated outputs to (consumer) decision blocks
#pragma HLS STREAM variable=nxbus_outputs depth=1

   // disable warning in GCC for anonymous structs, like 'nxbus_to_decision'
   #pragma GCC diagnostic ignored "-Wlocal-type-template-args"
   struct nxbus_to_decision {} ;
   typedef enyx::hls_tools::demuxer<nxbus_to_decision, strategy_count+1, nxmd::nxbus_axi>  nxbus_to_decision_demuxer_type; // create demuxer/duplicate type
   nxbus_to_decision_demuxer_type::p_demux(nxbus_in, nxbus_outputs); // effectively demux/duplicate

   // Mux/arbitrate the order trigger commands from the various Algorithms
   struct decisions_to_trigger {};
   typedef enyx::hls_tools::arbiter<decisions_to_trigger, strategy_count+1, nxoe::trigger_command_axi>  decisions_to_trigger_arbiter_type; // create arbiter type

   static hls::stream<nxoe::trigger_command_axi> decisions_ouputs[strategy_count+1]; // duplicated outputs, consumed by decision blocks
#pragma HLS STREAM variable=decisions_ouputs depth=1

   decisions_to_trigger_arbiter_type::p_arbitrate(decisions_ouputs, trigger_bus_out);

   // Top of Book Read & Write Buses
   static hls::stream<nxmd::BooksData<strategy_count,instrument_count>::halfbook_entry_update_request> book_update_bus; /// transport books updates
   static hls::stream<nxmd::BooksData<strategy_count,instrument_count>::read_book_data_request> read_book_request_bus[strategy_count]; /// transports read book requests
   static hls::stream<nxmd::BooksData<strategy_count,instrument_count>::book_entry> books[strategy_count]; /// transport read books entries
#pragma HLS STREAM variable=book_update_bus depth=1
#pragma HLS STREAM variable=read_book_request_bus depth=1
#pragma HLS STREAM variable=books depth=1

   // contextual data to take a trigger decision
   static hls::stream<enyx::oe::nxaccess_hw_algo::Tick2cancel::ContextData> t2c_context;

   // User notification DMA channel : need to arbitrate between configuration module and strategies
   struct dma_notifications{};
   typedef enyx::hls_tools::arbiter<dma_notifications, strategy_count +1, enyx::hfp::dma_user_channel_data_out>  dma_notification_arbiter_type;
   // static hls::stream<dma_user_channel_data_out> notifications_to_cpu[strategy_count+1]; /// transports algo notification to the DMA
   // #pragma HLS STREAM variable=notifications_to_cpu depth=1
   // dma_notification_arbiter_type::p_arbitrate(notifications_to_cpu, user_dma_channel_data_out);

   // data buses for notifications; as notifying to the DMA can be a long process, we set 32 items as depth for these FIFOs
   static hls::stream<algo::user_dma_update_instrument_configuration_ack> config_to_notifs;
   #pragma HLS STREAM variable=config_to_notifs depth=4
   static hls::stream<algo::user_dma_tick2trade_notification> tick2trade_to_notifs;
   #pragma HLS STREAM variable=tick2trade_to_notifs depth=4
   static hls::stream<algo::user_dma_tick2cancel_notification> tick2cancel_to_notifs;
   #pragma HLS STREAM variable=tick2cancel_to_notifs depth=4
   static hls::stream<algo::user_dma_tcp_consumer_notification> tcp_to_notifs;
   #pragma HLS STREAM variable=tcp_to_notifs depth=4


   /// Tick to Cancel Algorithm
   // process nxbus, make requests to books & instruments data
   enyx::oe::nxaccess_hw_algo::Tick2cancel::preprocess_nxbus(nxbus_outputs[Tick2Cancel],
                                                             instrument_read_bus[Tick2Cancel],
                                                             read_book_request_bus[Tick2Cancel],
                                                             t2c_context);
   // process response from book & instruments data, perform trigger
   enyx::oe::nxaccess_hw_algo::Tick2cancel::trigger(instrument_read_responses[Tick2Cancel],
                                                    books[Tick2Cancel],
                                                    decisions_ouputs[Tick2Cancel],
                                                    tick2cancel_to_notifs,
                                                    t2c_context);


   // Price Collar Algorithm
   enyx::oe::nxaccess_hw_algo::Tick2trade::p_algo(nxbus_outputs[1],
                           instrument_read_bus[1],
                           instrument_read_responses[1],
                           decisions_ouputs[1],
                           tick2trade_to_notifs,
                           read_book_request_bus[1],
                           books[1]);


    // Book Update Process: uses nxbus, and update book memory
    enyx::md::hw::BooksData<strategy_count,instrument_count>::p_book_updates(nxbus_outputs[2],
                                                                            book_update_bus);

    // Dispatch book memory to the various strategies
    enyx::md::hw::BooksData<strategy_count,instrument_count>::p_book_requests(book_update_bus,
                                                                            read_book_request_bus,
                                                                            books);

    // Store instrument configuration received from SW & provide it to the other functions
    algo::InstrumentConfiguration::p_handle_instrument_configuration(user_dma_channel_data_in,
                                                                       instrument_read_bus,
                                                                       instrument_read_responses,
                                                                       config_to_notifs); 

     // Handle notifications from workers to DMA 
     algo::Notifications::p_broadcast_notifications(tick2cancel_to_notifs, 
                                                   tick2trade_to_notifs, 
                                                   config_to_notifs,
                                                   tcp_to_notifs,
                                                   user_dma_channel_data_out);


     // Consumes TCP input data, and trigger
     enyx::oe::nxaccess_hw_algo::TcpConsumer::p_consume_tcp(
        tcp_replies_in,
        tcp_to_notifs,
        decisions_ouputs[strategy_count]);
}
