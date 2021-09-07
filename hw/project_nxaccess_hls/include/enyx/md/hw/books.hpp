//--------------------------------------------------------------------------------
//--! Licensed Materials - Property of ENYX
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
#include <iostream>
#include <ap_int.h>
#include <hls_stream.h>

#include <stdint.h>

#include "nxbus.hpp"
namespace enyx {
namespace md {
namespace hw {

/// Container class/core for storing books top level data
template <unsigned int ClientCount = 2, unsigned int InstrumentCount = 256>
class BooksData
{
  public:
    static std::size_t const instrument_count = InstrumentCount;

    // read book request
    typedef uint32_t read_book_data_request ; /// read book data request in memory

    // update book request
    struct halfbook_entry_update_request {
        ap_uint<32> book_index;
        ap_uint<1> side; //buy_nsell:  buy = 1, sell = 0
        ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> toplevel_price;
        ap_uint<8> uncross_depth; // uncross depth information from the PBU feature
    };

    /// memory structure used for storing instrument configuration
    /// layout of the packet sent by software to FPGA
    /// see messages.hpp for message layout reference
    struct halfbook_entry {
        //constructors
        halfbook_entry(ap_uint<1> _present, ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> _price):
               present(_present), toplevel_price(_price) {}
        halfbook_entry() {}
        //data
        ap_uint<1> present;
        ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> toplevel_price;
    };

    struct book_entry {
        //constructors
        book_entry() {}
        book_entry( ap_uint<1> _ask_present, ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> _ask_toplevel_price,
                    ap_uint<1> _bid_present, ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> _bid_toplevel_price):
            ask_present(_ask_present), ask_toplevel_price(_ask_toplevel_price),
            bid_present(_bid_present), bid_toplevel_price(_bid_toplevel_price) {}
        //data
        ap_uint<1> ask_present;
        ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> ask_toplevel_price;
        ap_uint<1> bid_present;
        ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> bid_toplevel_price;
    };

    BooksData() {}

public: // public data

    /// Read book request from other functions and answer to it
    static void
    p_book_requests(hls::stream<BooksData::halfbook_entry_update_request> & update_halfbook,
                                  hls::stream<BooksData::read_book_data_request> (& req_read_book_req_in)[ClientCount],
                                  hls::stream<BooksData::book_entry> (& read_book_req_out)[ClientCount])
    {

        /// Stores books data : half[0][x] is Sell side, half[1][x] is Buy side
        static halfbook_entry books_data[2][BooksData<ClientCount,InstrumentCount>::instrument_count];
        // split in at least 2 memories, one for each side
        #pragma HLS ARRAY_PARTITION variable=books_data block factor=2 dim=1

        #pragma HLS INLINE recursive
        #pragma HLS PIPELINE enable_flush
        if(!update_halfbook.empty()) { // incoming request for update
            // get update request data : book index, price & side
            halfbook_entry_update_request const request = update_halfbook.read();
            // update memory with halfbook
            books_data[request.side][request.book_index] = halfbook_entry(1, request.toplevel_price);
            return;
        }

         // process first the memory resquest for min latency
        for (int i = 0; i != ClientCount ; ++ i) {
            if(!req_read_book_req_in[i].empty()) { // if we got some input, read memory & output
                //read request
                read_book_data_request const book_index_req = req_read_book_req_in[i].read();
                //read data from memory, latency is here. Both memories are splitted
                halfbook_entry const sell = books_data[0][book_index_req];
                halfbook_entry const buy  = books_data[1][book_index_req];
                // prepare output
                book_entry output = book_entry(sell.present, sell.toplevel_price,
                                               buy.present, buy.toplevel_price);
                read_book_req_out[i].write(output);
                // break; // we only process a book request at a time
            }
        }
    } // p_book_requests

    static void
    p_book_updates(hls::stream<nxbus_axi> & nxbus_in,
                   hls::stream<BooksData::halfbook_entry_update_request> & book_update_request_out)
    {
        #pragma HLS INLINE recursive
        #pragma HLS PIPELINE enable_flush

        static bool is_end_of_extra = true; // assume previous message contains end of extra
        #pragma HLS RESET variable=is_end_of_extra

        static uint8_t current_opcode = 0x00; // opcode of the current nxbus command (in case of multi-cycle commands)
        #pragma HLS RESET variable=current_opcode

        static BooksData<2,256>::halfbook_entry_update_request output;
        #pragma HLS RESET variable=output

        // Local variables
        nxbus_axi nxbus_data_in;
        //TODO check if these pragmas are really required.
        #pragma HLS DATA_PACK variable=nxbus_data_in
        nxbus nxbus_word_in;
        #pragma HLS DATA_PACK variable=nxbus_word_in

        if (! nxbus_in.empty()) {
            nxbus_data_in = nxbus_in.read();
            nxbus_word_in = static_cast<nxbus>(nxbus_data_in);
            if (is_end_of_extra) { // only process nxbus messages with EoE flag
                current_opcode = nxbus_word_in.opcode;
                if ((nxbus_word_in.opcode == NXBUS_OPCODE_BOOK_UPDATE) &&
                        (nxbus_word_in.data2(7,0) == 0)) { //only keep level 0 of buy or sell side

                    std::cout << "[DECISION][book_updater] [nxbus timestamp " << std::hex << nxbus_word_in.timestamp << "] "
                                << "Updating book for instrument : " << nxbus_word_in.instr_id
                                << " price=" << nxbus_word_in.price
                                << " side=" << nxbus_word_in.buy_nsell
                                << std::endl;
                    //BooksData<2,256>::halfbook_entry_update_request output = BooksData<2,256>::halfbook_entry_update_request();
                    output.book_index = nxbus_word_in.instr_id;
                    output.side = nxbus_word_in.buy_nsell;
                    output.toplevel_price = nxbus_word_in.price;
                    output.uncross_depth = 0x00;
                    if (nxbus_word_in.end_of_extra) {
                        book_update_request_out.write(output);
                    }
                }
            } else {
                // extra-cycle word
                if (current_opcode == NXBUS_OPCODE_BOOK_UPDATE) {
                    // Capture the uncross depth value (HKEX specific)
                    output.uncross_depth = nxbus_word_in.order_id(48-1, 40);

                    std::cout << "[DECISION][book_updater] [uncross_depth " << std::hex << output.uncross_depth << "] "
                                << std::endl;

                    if (nxbus_word_in.end_of_extra) {
                        book_update_request_out.write(output);
                    }
                }
            }

            is_end_of_extra = nxbus_word_in.end_of_extra; // keep EoE value for next
        }
    } // p_book_updates

}; // class BooksData
}}} // Namespaces

