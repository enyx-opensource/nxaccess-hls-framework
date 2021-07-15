#include <array>
#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <enyx/utils/compiler-dependencies.hpp>
#include <enyx/utils/data_format/HexString.hpp>

#include <enyx/oe/hwstrat/demo/Helper.hpp>

namespace demo = enyx::oe::hwstrat::demo;
namespace data_format = enyx::utils::data_format;

const std::string stream_name = "trigger-commands";
std::atomic_bool stop{false};

void signal_handler(int) {
    stop = true;
}

void usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name  << " accelerator_id\n"
        << "Read data sent from the FPGA to the trigger command bus on a standalone test firmware\n";
}

// Expected Trigger messages.
struct ENYX_PACKED_STRUCT TriggerOut {
    using Data = std::array<uint8_t, 16>;
    uint16_t  collection_id;
    uint8_t   reserved;
    uint8_t   parameter_mask;
    Data      data0;
    Data      data1;
    Data      data2;
    Data      data3;
    Data      data4;
};
static_assert(sizeof(TriggerOut) == 84, "Invalid Trigger Out size");

std::string
to_hex_string(uint8_t v) {
    return data_format::toHexString(v);
}

std::string
to_hex_string(uint16_t v) {
    std::stringstream ret;
    ret << std::hex << std::setw(4) << std::setfill('0') << v;
    return ret.str();
}


std::string
to_hex_string(const std::array<uint8_t, 16>& v) {
    std::string ret;
    for (std::size_t i = 0; i < v.size() ; ++i ) {
        const auto element = v.at(i);
        ret += to_hex_string(element);
        // Adding a space between the two 64 bits words.
        if (i == 7) { ret += " "; }
    }
    return ret;
}

void printer(const std::uint8_t * data, std::uint32_t size) {
    // Not the message layout we're expecting => print without formating
    if (size != sizeof(TriggerOut)) {
        std::cout << data_format::toHexString({data, size}) << "\n";
        return;
    }
    const auto msg = reinterpret_cast<const TriggerOut*>(data);
    std::cout << to_hex_string(ntohs(msg->collection_id)) << " "
              << to_hex_string(msg->parameter_mask) << " "
              // Reserved not printed
              << to_hex_string(msg->data0) << " "
              << to_hex_string(msg->data1) << " "
              << to_hex_string(msg->data2) << " "
              << to_hex_string(msg->data3) << " "
              << to_hex_string(msg->data4)
              << "\n";
}

int main(int argc, char ** argv) {

    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const std::size_t accelerator_id = std::atoi(argv[1]);

     std::signal(SIGINT, &signal_handler);
     std::signal(SIGTERM, &signal_handler);

    try {
        auto accelerator = demo::find_accelerator(accelerator_id);
        auto a2c_stream = demo::find_a2c_stream(accelerator, stream_name);
        auto poller = a2c_stream.get_poller(printer);
        while (not stop) {
            poller.poll_once();
        }
    } catch (std::exception& e) {
        std::cerr << "Unexpected exception caught: " << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}