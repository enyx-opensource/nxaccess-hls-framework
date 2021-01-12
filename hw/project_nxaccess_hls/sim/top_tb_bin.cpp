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
        while (! dma_data_in.empty()) {
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
            std::cout << "[TB] " << std::dec << 
                "iteration #" << iteration << ": " <<
                "dma_data_in: " << dma_data_in.size() << " words remaining" <<
                std::endl;
            ++iteration;
        }
        assert(dma_data_in.empty() && "Failed to upload all settings into the HLS module");

        std::cout << "[TB] Loading market data contents" << std::endl;

        read_nxbus_from_file(nxbus_in, generate_filename("nxbus_in", ".ref", Index, burst_index));
        std::cout << "[TB] Loaded " << std::dec << nxbus_in.size() << " nxbus packets" << std::endl;
        
        std::cout << "[TB] Loading TCP reply data contents" << std::endl;
        
        read_tcp_from_files(
            tcp_replies_in,
            "top_tb_tcp_bin/tcp_reply_session.ref.txt",
            "top_tb_tcp_bin/tcp_reply_data.ref.txt");

        std::cout << "[TB] Loaded " << std::dec << tcp_replies_in.size() << " TCP ingress words" << std::endl;

        std::cout << "[TB] Running kernel" << std::endl;

        int input_stimuli_count = std::max(nxbus_in.size(), tcp_replies_in.size());
        for (int i =0; i < input_stimuli_count; ++i) {
            std::cout << "[TB] Main processing loop iteration#" << std::dec << i << std::endl;
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
            std::cout << "[TB] " << std::dec
                << "remaining market data: " << nxbus_in.size() << " words, "
                << "remaining TCP payload: " << tcp_replies_in.size() << " words, "
                << "output DMA: " << dma_data_out.size() << " words, "
                << "output trigger: " << trigger_out.size() << " events, "
                << std::endl;
        }
        assert(nxbus_in.empty() && "HLS module failed to sink all market data words");
        assert(tcp_replies_in.empty() && "HLS module failed to sink all TCP reply words");

        const int TOTAL_ALGORITHM_EXPECTED_LATENCY = 10;
        for(int i = 0 ; i < TOTAL_ALGORITHM_EXPECTED_LATENCY; ++i)
        {
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, tcp_replies_in);
            std::cout << "[TB] " << std::dec
                << "remaining market data: " << nxbus_in.size() << " words, "
                << "remaining TCP payload: " << tcp_replies_in.size() << " words, "
                << "output DMA: " << dma_data_out.size() << " words, "
                << "output trigger: " << trigger_out.size() << " events, "
                << std::endl;
        }

        // generates trigger output to file. consumes trigger_out bus
        auto triggered = dump_trigger_to_file(trigger_out, generate_filename("trigger_out", ".gen", Index, burst_index));
        auto trigger_ref = read_trigger_from_file(generate_filename("trigger_out", ".ref", Index, burst_index));
        compare_generated_and_reference(trigger_ref, triggered);

        // throw std::invalid_argument("stop L98");
        // std::cout << "[TEST] Comparing trigger output with ref file...\n";
        // std::cout << "[TEST] Trigger output compared successfully ! \n";

        // Flush the DMA out, as we don't dump it, nor test it.
        while(!dma_data_out.empty()) {
            dma_data_out.read();
        }

        // std::cout << "\tBurst " << burst_index << " End" << std::endl;

        std::cout << "[TB] Completed." << std::endl;
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

    template<typename T>
    static void
    fill_tcp_stream(
        hls::stream<T> & stream,
        ap_uint<8> session,
        std::string const & content,
        ap_uint<20> user)
    {
        
        T stream_word = T();
        stream_word.id = session;
        stream_word.keep = 0xFFFF;
        stream_word.data = 0;
        stream_word.user = 0;
        
        std::istringstream iss (content);
        std::string hexbyte_str;

        const std::size_t word_byte_count = T::data_width / 8;
        const std::size_t content_byte_count = content.size() / 2;
        bool last = false;

        std::size_t i = 0;
        while ((iss >> std::setw(2) >> hexbyte_str)) {
            std::istringstream hexbyte_iss (hexbyte_str);
            uint16_t hexbyte_u16; // 8-bit types will not work for some reason
            hexbyte_iss >> std::setbase(16) >> hexbyte_u16;

            stream_word.data <<= 8;
            stream_word.data(8-1, 0) = hexbyte_u16;

            ++i;
            if ((i % word_byte_count) == 0) {
                last = (i == content_byte_count);
                stream_word.last = last;

                if (last)
                    stream_word.user = user;

                std::cout << "[TB] pushed TCP word: " << std::hex
                    << "id: " << std::setw(2) << stream_word.id << ", "
                    << "bytes: " << stream_word.data << ", "
                    << "last: " << std::dec << stream_word.last << std::hex << ", "
                    << "keep: " << stream_word.keep << ", "
                    << "user: " << stream_word.user << ", "
                    << std::endl;

                stream.write(stream_word);
                stream_word.data = 0;
                stream_word.user = 0;
            }
        }

        if (! last) {
            // handle data packet with null bytes on eop
            const std::size_t remaining_bytes = content_byte_count % word_byte_count;
            assert(remaining_bytes && "expected at least one trailing byte");

            for (auto i = remaining_bytes; i < word_byte_count; i++) {
                const uint8_t null_byte = 0xCD;
                stream_word.keep.clear(word_byte_count -1 - i);

                stream_word.data <<= 8;
                stream_word.data(8-1, 0) = null_byte;
            }

            last = true;
            stream_word.last = last;
            stream_word.user = user;

            std::cout << "[TB] pushed TCP word: " << std::hex
                << "id: " << std::setw(2) << stream_word.id << ", "
                << "bytes: " << stream_word.data << ", "
                << "last: " << std::dec << stream_word.last << std::hex << ", "
                << "keep: " << stream_word.keep << ", "
                << "user: " << stream_word.user << ", "
                << std::endl;

            stream.write(stream_word);
        }
        assert(last && "missing end of packet");
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
            bool session_eof = true;
            bool content_eof = true;
            ap_uint<8> session;
            ap_uint<8> tcp_error;

            std::string content;

            std::string session_line;
            for (std::string session_line; std::getline(packet_sessions, session_line); ) { 
                if (!session_line.empty() && session_line[0] != '#') {
                    std::istringstream ss(session_line);
                    session =  enyx::get_from_hex_stream_as< ap_uint<8> >(ss);
                    tcp_error = enyx::get_from_hex_stream_as< ap_uint<8> >(ss);
                    std::cout << "[TB] " << std::dec
                        << "session: " << session << ", "
                        << "tcp_error: " << tcp_error << ", "
                        << std::endl;

                    session_eof = false;
                    break;
                }
                session_eof = true;
            }
            //assert(false);
            std::string content_bytes;
            for (std::string content_bytes; std::getline(packet_bytes, content_bytes); ) { 
                if (!content_bytes.empty() && content_bytes[0] != '#') {
                    content = std::string(content_bytes);
                    content_eof = false;
                    break;
                }
                content_eof = true;
            }

            if (content_eof)
                break;

            std::cout << "~~~~ content bytes: " << content << std::endl;
            fill_tcp_stream(data_in, session, content, tcp_error);
        }
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
