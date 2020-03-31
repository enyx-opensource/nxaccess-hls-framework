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

#include <math.h>
#include <stdint.h>
#include <stdexcept>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#include "../include/enyx/hls/helpers.hpp"
#include "../include/enyx/hls/string.hpp"

#include "../include/enyx/md/hw/nxbus.hpp"
#include "../include/enyx/md/hw/string.hpp"

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/oe/hwstrat/string.hpp"

#include "../src/top.hpp"
#include "../src/configuration.hpp"
#include "../src/messages.hpp"
#include "../include/enyx/hfp/hfp.hpp"
#include "../include/enyx/oe/hwstrat/tcp.hpp"

namespace nxoe = enyx::oe::hwstrat;
using _nxbus = enyx::md::hw::nxbus_axi;
using _trigger_cmd = enyx::oe::hwstrat::trigger_command_axi;

template<std::size_t Index, std::size_t BurstCount>
class TopTestBench
{
public:
    static std::size_t const CYCLES_PER_MSG = 80;

private:

public:
    TopTestBench()
    {
        std::cout << ">>> Top Test #" << Index << " Begin" << std::endl;

        for (std::size_t i = 0; i != BurstCount; ++i)
            test_burst(i);

        std::cout << "<<< Top Test #" << Index << " End" << std::endl;
    }

private:
    void
    test_burst(std::size_t burst_index)
    {
        std::cout << "\t*** Burst #" << burst_index << " Begin" << std::endl;

        hls::stream<_nxbus>                 nxbus_in("nxbus_in");
        hls::stream<_trigger_cmd>           trigger_out("trigger_command");
        hls::stream<nxoe::tcp_reply_payload>      tcp_replies_in("tcp_replies");
        hls::stream<enyx::hfp::dma_user_channel_data_in>           dma_data_in("dma_user_channel_data_in");
        hls::stream<enyx::hfp::dma_user_channel_data_out>          dma_data_out("dma_user_channel_data_out");

        std::cout << "[TB] Loading DMA configuration" << std::endl;

        read_dma_config_in_from_file(dma_data_in, generate_filename("dma_user_in", ".ref", Index, burst_index));
        std::cout << "[TB] Loaded " << std::dec << dma_data_in.size() << " configuration commands" << std::endl;
        int iteration = 0;
        while (dma_data_in.size() > 0) {
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
            std::cout << "[TB] " << std::dec << 
                "iteration #" << iteration << ": " <<
                "dma_data_in: " << dma_data_in.size() << " words remaining" <<
                std::endl;
            ++iteration;
        }

        std::cout << "[TB] Loading data contents" << std::endl;

        read_nxbus_from_file(nxbus_in, generate_filename("nxbus_in", ".ref", Index, burst_index));
        std::cout << "[TB] Loaded " << std::dec << nxbus_in.size() << " nxbus packets" << std::endl;
        read_tcp_from_files(
            tcp_replies_in,
            "top_tb_tcp_bin/tcp_reply_session.ref.txt",
            "top_tb_tcp_bin/tcp_reply_data.ref.txt");
        throw std::invalid_argument("stop ~~");

        // Process ctrl.
        std::cout << "[TEST] Running on triggers...\n";
        // while there's some input in TCP or nxBus, run the algorithm
        int input_stimuli_count = std::max(nxbus_in.size(), tcp_replies_in.size());
        for (int i =0; i < input_stimuli_count; ++i) {
            std::cout << "[TB] Main processing loop iteration#" << std::dec << i << "\n";
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
        }

        




        // ensure all entries where consumed, if not, there's a problem. For instance, some backpressure could be
        // a legitimate reason
        assert(nxbus_in.empty());
        assert(tcp_replies_in.empty()); // ensure all entries where consumed, if not, there's a problem.

        const int TOTAL_ALGORITHM_EXPECTED_LATENCY = 10;
        for(int i = 0 ; i < TOTAL_ALGORITHM_EXPECTED_LATENCY; ++i)
        {
            std::cout << "[TB] Post processing loop iteration#" << std::dec << i << "\n";
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
        }

        {
            // generates trigger output to file. consumes trigger_out bus
            auto triggered = dump_trigger_to_file(trigger_out, generate_filename("trigger_out", ".gen", Index, burst_index));
            auto trigger_ref = read_trigger_from_file(generate_filename("trigger_out", ".ref", Index, burst_index));
            std::cout << "[TEST] Comparing trigger output with ref file...\n";
            compare_generated_and_reference(trigger_ref, triggered);
            std::cout << "[TEST] Trigger output compared successfully ! \n";
        }

        // Flush the DMA out, as we don't dump it, nor test it.
        while(!dma_data_out.empty()) {
            dma_data_out.read();
        }


        std::cout << "\tBurst " << burst_index << " End" << std::endl;
    }

    /// Give a filename from prefix, index & burst for testbench input & output
    static std::string
    generate_filename(std::string const& prefix, std::string const &suffix, std::size_t index, std::size_t burst)
    {
        std::ostringstream out;
        out << "top_tb_tcp_bin/" << prefix << "_" << burst << suffix << ".txt";
        std::cout << "[TB] ~~~: " << "top_tb_tcp_bin/" << prefix << "_" << burst << suffix << ".txt" << std::endl;
        return out.str();
    }

    
    static void
    read_nxbus_from_file(hls::stream<enyx::md::hw::nxbus_axi> & data_in, std::string const& file)
    {
        std::ifstream data_in_file(file.c_str());
        assert(data_in_file);
        for (std::string l; std::getline(data_in_file, l); )
            if (! l.empty() && l[0] != '#')
                enyx::md::hw::convert_nxbus_string_to_nxbus_axi(data_in, l);

    }

    static void
    read_dma_config_in_from_file(hls::stream<enyx::hfp::dma_user_channel_data_in> & data_in, std::string const& file)
    {
        std::ifstream data_in_file(file.c_str());
        assert(data_in_file);
        for (std::string l; std::getline(data_in_file, l); ) {
            if (! l.empty() && l[0] != '#') {
                 convert_string_to_dma_channel_in(data_in,l);
            }
        }
    }

    /**
     * @brief read_tcp_reply_in_from_file Fills a stream
     * @param data_in
     * @param file
     */
    static void read_tcp_reply_in_from_file(hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & data_in,
                                            std::string const& filename)
    {
        std::ifstream tcp_in(filename.c_str());
        assert(tcp_in && "Can't open file !" );
        for (std::string l; std::getline(tcp_in, l); ) {
            if (! l.empty() && l[0] != '#') {
                 tcp_reply_reply_payload_convert_tb(data_in,l);
            }
        }
    }



// convert_nxbus_string_to_nxbus_axi(hls::stream<enyx::md::hw::nxbus_axi> & result, std::string const& content)
// {
//     // We want to read this kind of line: 
//     // # EOE|id|code|order_id   |buy|qty    | price          | timestamp| instr_ascii                  | instr_bin|instr_id|data0         | data1  | data2
//     // 01 00 95 0000000000000000 00 00000030 13A9875F0CBF7981 00000000 00000000000000000000000000000000 00000000 00000002 0102030405060708 00000000 0000000000000000

//     std::istringstream ss(content);
//     enyx::md::hw::nxbus nxbus_word;
    
//     // Beware, do not use uint8_t as storage, as C++ stringstream treats this as 'char
//     nxbus_word.end_of_extra              = enyx::get_from_hex_stream_as<uint32_t>(ss);
//     nxbus_word.market_internal_id        = enyx::get_from_hex_stream_as<uint32_t>(ss);
//     nxbus_word.opcode                    = enyx::get_from_hex_stream_as<uint32_t>(ss);
//     nxbus_word.order_id                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.buy_nsell                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.qty                       = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.price                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.timestamp                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.instr_ascii               = enyx::get_from_hex_stream_as<uint64_t>(ss); // FIXME most probably wrong as doesn't fit on 64bits
//     nxbus_word.instr_bin                 = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.instr_id                  = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.data0                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.data1                     = enyx::get_from_hex_stream_as<uint64_t>(ss);
//     nxbus_word.data2                     = enyx::get_from_hex_stream_as<uint64_t>(ss);

//     result.write(static_cast<enyx::md::hw::nxbus_axi>(nxbus_word));
// }

    template<typename Word>
    static void
    fill_tcp_stream(hls::stream<Word> & stream, ap_uint<8> session, std::string const & content)
    {
        static std::size_t const word_byte_count = Word::data_width / 8;
        Word word = Word();
        word.id = session;
        word.data = 0;

        std::istringstream iss(content);

        for (std::size_t i = 0, words = content.size(); 2*i < words; ) {
            // iterate over each byte

            word.data <<= 8;
            word.data(8-1, 0) = enyx::get_from_hex_stream_as< ap_uint<8> >(iss);

            ++i;
            if (i % word_byte_count == 0) {
                word.last = (i == content.size() - 1);
                stream.write(word);
                word.data = 0;
            }
        }

        std::size_t const remaining_byte_count = content.size() % word_byte_count;
        if (remaining_byte_count)
        {
            word.data <<= (word_byte_count - remaining_byte_count) * 8;
            word.last = 1;
            stream.write(word);
        }
    }

    /**
     * @brief read_tcp_from_files Fills a stream
     * @param data_in
     * @param session_filename
     * @param data_filename
     */

    static void
    read_tcp_from_files(
        hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & data_in,
        std::string const & session_filename,
        std::string const & data_filename)
    {
        std::ifstream packet_sessions(session_filename.c_str());
        std::ifstream packet_bytes(data_filename.c_str());
        assert(packet_sessions && packet_bytes && "missing stimuli files" );

        while (true) {
            //enyx::oe::hwstrat::tcp_reply_payload tcp_word;
            ap_uint<8> session;

            std::string session_line;
            for (std::string session_line; std::getline(packet_sessions, session_line); ) { 
                //std::cout << "~~~~ session: " << session_line << std::endl;
                if (!session_line.empty() && session_line[0] != '#') {
                    std::cout << "~~~~ session data: " << session_line << std::endl;
                    std::istringstream ss(session_line);
                    session =  enyx::get_from_hex_stream_as< ap_uint<8> >(ss);
                    break;
                }
            }

            std::string content_bytes;
            for (std::string content_bytes; std::getline(packet_bytes, content_bytes); ) { 
                //std::cout << "~~~~ session: " << content_bytes << std::endl;
                if (!content_bytes.empty() && content_bytes[0] != '#') {
                    std::cout << "~~~~ content bytes: " << content_bytes << std::endl;
                    break;
                }
            }

            fill_tcp_stream(data_in, session, content_bytes);
        }
        throw std::invalid_argument("stop ~~");

//   while(true)
//   {
//     fil1.read((char*)&e,sizeof(e));
//     // do the checking right after a read()
//     if (fil1.eof())
//       break;
//     e.display1();

//     fil1.read((char*)&s,sizeof(s));
//     s.display2();
//   }   

        // for (std::string l; std::getline(tcp_in, l); ) {
        //     if (! l.empty() && l[0] != '#') {
        //          tcp_reply_reply_payload_convert_tb(data_in,l);
        //     }
        // }
    }

    /// Reads words of DMA from file
    static void
    read_data_in_from_file(hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & data_in, std::string const& file)
    {
        std::ifstream data_in_file(file.c_str());
        assert(data_in_file);
        for (std::string l; std::getline(data_in_file, l); )
            if (! l.empty() && l[0] != '#')
                enyx::fill_stream_with_text(data_in, l);
    }


    static void
    convert_string_to_cpu2fpgaheader(enyx::oe::hwstrat::cpu2fpga_header & out,  std::istringstream& in)
    {
        out.version =  enyx::get_from_hex_stream_as<uint16_t>(in);
        std::cout << "[VERBOSE] out.version: " << std::dec << out.version << std::endl;

        out.dest =  enyx::get_from_hex_stream_as<uint16_t>(in);
        std::cout << "[VERBOSE] out.dest: " << std::dec << out.dest << std::endl;

        out.msg_type =  enyx::get_from_hex_stream_as<uint16_t>(in);
        std::cout << "[VERBOSE] out.msg_type: " << std::dec << out.msg_type << std::endl;

        out.ack_request =  enyx::get_from_hex_stream_as<uint16_t>(in);
        std::cout << "[VERBOSE] out.ack_request: " << std::dec << out.ack_request << std::endl;

//        out.reserved =  enyx::get_from_hex_stream_as<uint16_t>(in);
//        std::cout << "[VERBOSE] out.reserved: " << std::dec << out.reserved << std::endl;

        out.timestamp =  enyx::get_from_hex_stream_as<uint32_t>(in);
        std::cout << "[VERBOSE] out.timestamp: " << std::dec << out.timestamp << std::endl;

        out.length =  enyx::get_from_hex_stream_as<uint32_t>(in);
        std::cout << "[VERBOSE] out.length: " << std::dec << out.length << std::endl;
    }

    /// Converts strings representing DMA inputs (instrument configurations) to 128b words (enyx::hfp::dma_user_channel_data_in)
    static void
    convert_string_to_dma_channel_in(hls::stream<enyx::hfp::dma_user_channel_data_in> & result, std::string const& content)
    {
        // We want to read :
//        # cpu2fpga_header   | tick_to_cancel_threshold | tick_to_trade_bid_price | tick_to_trade_ask_price |  tick_to_trade_bid_collection_id | tick_to_cancel_collection_id | tick_to_trade_ask_collection_id | instrument_id|enable
//        # version 1, module 8, msgtype 1 , ack request = 0 , reserved = 0, timestamp 0x42, length unused yet
//        01 08 01 00 0 42 00   00000004A817C800           0000000000000000           0000000000000000          0010                               0011                          0012                                0014         1

        enyx::hfp::dma_user_channel_data_out word1;
        enyx::hfp::dma_user_channel_data_out word2;
        enyx::hfp::dma_user_channel_data_out word3;
        enyx::hfp::dma_user_channel_data_in word;

        enyx::hfp::dma_user_channel_data_in word_;
        enyx::hfp::dma_user_channel_data_in word__;
        enyx::oe::nxaccess_hw_algo::user_dma_update_instrument_configuration tmp;

        std::istringstream ss(content);

        // read cpu2fpga_header
        convert_string_to_cpu2fpgaheader(tmp.header, ss);

        tmp.tick_to_cancel_threshold =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.tick_to_trade_bid_price =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.tick_to_trade_ask_price =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.instrument_id =  enyx::get_from_hex_stream_as<uint32_t>(ss);
        tmp.tick_to_trade_bid_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);
        tmp.tick_to_cancel_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);
        tmp.tick_to_trade_ask_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);

        tmp.enabled =  enyx::get_from_hex_stream_as<uint16_t>(ss);

        // convert input DMA message to 3 words as it would come into the FPGA
        for(int i = 1; i <= 3; ++i) 
        {
            enyx::hfp::dma_user_channel_data_out word;
            enyx::hfp::dma_user_channel_data_in out;
            enyx::oe::nxaccess_hw_algo::InstrumentConfiguration::write_word(tmp, word, i);
            out.data(127,0) = word.data(127,0);
            out.last = word.last;
            result.write(out);
        }
    }

    /// Converts strings representing DMA inputs (instrument configurations) to 128b words (enyx::hfp::dma_user_channel_data_in)
    static void
    tcp_reply_reply_payload_convert_tb(hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & result,
                                       std::string const& content)
    {
        //# TCP reply payloads from the market.
        //#--------------------------------------
        //#8b             128b.
        //#XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX X
        //# |         |                        |
        //# |         |                        + last : 1 bit for last packet of a burst.
        //# |         +-----    TCP data payload
        //# |
        //# +----   TCP Session ID on 8 bits



        enyx::oe::hwstrat::tcp_reply_payload burst;
        std::istringstream ss(content);
        burst.user =  enyx::get_from_hex_stream_as< ap_uint<8> >(ss);
        burst.data =  enyx::get_from_hex_stream_as< ap_uint<128> >(ss);
        burst.last =  enyx::get_from_hex_stream_as< ap_uint<1> >(ss);

        std::cout << "[VERBOSE]" << std::hex <<  "tcp : " << "\n\tdata = " << burst.data << "\n\t user = " << burst.user << "\n";
        result.write(burst);
    }

    /// feeds trigger_command_axi stream from file
    static std::vector<std::string>
    read_trigger_from_file(std::string const& file)
    {
        std::vector<std::string> ret;
        std::ifstream data_in_file(file.c_str());
        assert(data_in_file);
        for (std::string l; std::getline(data_in_file, l); )
            if (! l.empty() && l[0] != '#')
                ret.push_back(l);
        return ret;
    }

    static void
    compare_generated_and_reference(std::vector<std::string> const& expected,
                   std::vector<std::string> const& generated)
    {
        if(expected.size() != generated.size())
        {
            std::cerr << "[TB] Comparing lists of messages but messages count different on both side ! " << "\n" <<
                       "\t : expected=" << expected.size() << " items  vs generated=" << generated.size() << " items\n";
        }

        for (std::size_t i = 0, e = expected.size(); i != e; ++i) {
            if(i<generated.size()) {
                ASSERT_EQ(expected[i], generated[i]);
            }
        }

        ASSERT_EQ(expected.size(), generated.size());

    }

    static std::vector<std::string>
    dump_trigger_to_file(hls::stream<enyx::oe::hwstrat::trigger_command_axi> & data_in, std::string const& file)
    {
        std::vector<std::string> ret;
        std::ofstream data_out_file(file.c_str());
        assert(data_out_file);
        data_out_file << nxoe::get_trigger_command_axi_file_format_header();
        int acc = 0;
        while(!data_in.empty()) {
            std::string data = nxoe::convert_trigger_command_axi_to_text(data_in);
            data_out_file << data << "\n";
            ret.push_back(data);
            ++acc;
        }

        std::cout << "[TB] Dumped " << std::dec << acc << " triggers to file. \n";
        return ret;
    }

    static void
    check_data_out(hls::stream<nxoe::trigger_command_axi> & data_out, std::string const& file)
    {
        while (! data_out.empty()) data_out.read();
    }
};

int
main(int argc, char** argv)
{

    TopTestBench<0, 1>();


    return 0;
}
