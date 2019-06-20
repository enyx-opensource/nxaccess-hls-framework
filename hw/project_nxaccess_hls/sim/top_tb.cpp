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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#include "../include/enyx/hls/helpers.hpp"
#include "../include/enyx/hls/string.hpp"

#include "../include/enyx/hfp/hls/hfp.hpp"

#include "../include/enyx/md/hw/nxbus.hpp"
#include "../include/enyx/md/hw/string.hpp"

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/oe/hwstrat/string.hpp"

#include "../src/top.hpp"
#include "../src/configuration.hpp"
#include "../src/messages.hpp"

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
        //hls::stream<nxoe::tcp_replies>      tcp_replies_in("tcp_replies");
        hls::stream<enyx::hfp::hls::dma_user_channel_data_in>           dma_data_in("dma_user_channel_data_in");
        hls::stream<enyx::hfp::hls::dma_user_channel_data_out>          dma_data_out("dma_user_channel_data_out");

        uint32_t instrument_count           = 0;


        // read reference file
        std::cout << "[TEST] Reading nxbus_in ref file...\n";
        read_nxbus_from_file(nxbus_in, generate_filename("nxbus_in", ".ref", Index, burst_index));
        read_dma_config_in_from_file(dma_data_in, generate_filename("dma_user_in", ".ref", Index, burst_index));


        assert(!nxbus_in.empty());


        // Process ctrl.
        std::cout << "[TEST] Running algorithm...\n";
        int acc = 0;

        while(nxbus_in.size() > 0 ) {
             ++acc;
            std::cout << "[TB] Main Processing loop iteration#" << std::dec << acc << "\n";
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, //tcp_replies_in,
                                 &instrument_count);
        }
        const int TOTAL_LATENCY = 10;
        for(int i = 0 ; i < TOTAL_LATENCY; ++i)
        {
            algorithm_entrypoint(nxbus_in, dma_data_in, dma_data_out, trigger_out, //tcp_replies_in,
                             &instrument_count);
        }

        while(!nxbus_in.empty())
        {
            std::cout << "Flushing remaining nxBus entry! \n";
            nxbus_in.read();
        }

        assert(nxbus_in.empty()); // ensure all entries where consumed, if not, there's a problem.
        {
            // generates trigger output to file. consumes trigger_out bus
            auto triggered = dump_trigger_to_file(trigger_out, generate_filename("trigger_out", ".gen", Index, burst_index));
            auto trigger_ref = read_trigger_from_file(generate_filename("trigger_out", ".ref", Index, burst_index));
            std::cout << "[TEST] Comparing trigger output with ref file...\n";
            compare_ref_and_gen(trigger_ref, triggered);
            std::cout << "[TEST] Trigger output compared successfully ! \n";
        }

        for(int i  = 0 ; i < 10; ++i) {
            while (!dma_data_out.empty())
                dma_data_out.read();
        }


        std::cout << "\tBurst " << burst_index << " End" << std::endl;
    }

    /// Give a filename from prefix, index & burst for testbench input & output
    static std::string
    generate_filename(std::string const& prefix, std::string const &suffix, std::size_t index, std::size_t burst)
    {
        std::ostringstream out;
        out << "top_tb_" << index << "/" << prefix << "_" << burst << suffix << ".txt";
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
    read_dma_config_in_from_file(hls::stream<enyx::hfp::hls::dma_user_channel_data_in> & data_in, std::string const& file)
    {
        std::ifstream data_in_file(file.c_str());
        assert(data_in_file);
        for (std::string l; std::getline(data_in_file, l); ) {
            if (! l.empty() && l[0] != '#') {
                 convert_string_to_dma_channel_in(data_in,l);
            }
        }
    }

    static void
    convert_string_to_cpu2fpgaheader(enyx::oe::hwstrat::cpu2fpga_header & out,  std::istringstream& in)
    {
        out.version =  enyx::get_from_hex_stream_as<uint16_t>(in);
        out.dest =  enyx::get_from_hex_stream_as<uint16_t>(in);
        out.msg_type =  enyx::get_from_hex_stream_as<uint16_t>(in);
        out.ack_request =  enyx::get_from_hex_stream_as<uint16_t>(in);
        out.reserved =  enyx::get_from_hex_stream_as<uint16_t>(in);
        out.timestamp =  enyx::get_from_hex_stream_as<uint32_t>(in);
        out.length =  enyx::get_from_hex_stream_as<uint32_t>(in);
    }

    /// Converts strings representing DMA inputs (instrument configurations) to 128b words (enyx::hfp::hls::dma_user_channel_data_in) 
    static void
    convert_string_to_dma_channel_in(hls::stream<enyx::hfp::hls::dma_user_channel_data_in> & result, std::string const& content)
    {
        // We want to read :
//        # cpu2fpga_header   | tick_to_cancel_threshold | tick_to_trade_bid_price | tick_to_trade_ask_price |  tick_to_trade_bid_collection_id | tick_to_cancel_collection_id | tick_to_trade_ask_collection_id | instrument_id|enable
//        # version 1, module 8, msgtype 1 , ack request = 0 , reserved = 0, timestamp 0x42, length unused yet
//        01 08 01 00 0 42 00   00000004A817C800           0000000000000000           0000000000000000          0010                               0011                          0012                                0014         1

        enyx::hfp::hls::dma_user_channel_data_out word1;
        enyx::hfp::hls::dma_user_channel_data_out word2;
        enyx::hfp::hls::dma_user_channel_data_out word3;
        enyx::hfp::hls::dma_user_channel_data_in word;

        enyx::hfp::hls::dma_user_channel_data_in word_;
        enyx::hfp::hls::dma_user_channel_data_in word__;
        enyx::oe::nxaccess_hw_algo::user_dma_update_instrument_configuration tmp;

        std::istringstream ss(content);

        // read cpu2fpga_header
        convert_string_to_cpu2fpgaheader(tmp.header, ss);

        tmp.tick_to_cancel_threshold =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.tick_to_trade_bid_price =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.tick_to_trade_ask_price =  enyx::get_from_hex_stream_as<uint64_t>(ss);
        tmp.tick_to_trade_bid_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);
        tmp.tick_to_cancel_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);
        tmp.tick_to_trade_ask_collection_id =  enyx::get_from_hex_stream_as<uint16_t>(ss);
        tmp.instrument_id =  enyx::get_from_hex_stream_as<uint32_t>(ss);

        tmp.enabled =  enyx::get_from_hex_stream_as<uint16_t>(ss);

        // convert input DMA message to 3 words as it would come into the FPGA
        for(int i = 1; i <= 3; ++i) 
        {
            enyx::hfp::hls::dma_user_channel_data_out word;
            enyx::hfp::hls::dma_user_channel_data_in out;
            enyx::oe::nxaccess_hw_algo::InstrumentConfiguration::write_word(tmp, word, i);
            out.data(127,0) = word.data(127,0);
            out.last = word.last;
            result.write(out);
        }
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
    compare_ref_and_gen(std::vector<std::string> const& expected,
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
        nxoe::convert_trigger_command_axi_to_text(data_in);
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
    TopTestBench<0, 2>();


    return 0;
}
