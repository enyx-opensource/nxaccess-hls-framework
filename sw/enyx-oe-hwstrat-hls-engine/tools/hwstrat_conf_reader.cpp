#include <array>
#include <atomic>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include <enyx/utils/compiler-dependencies.hpp>
#include <enyx/utils/data_format/HexString.hpp>

#include <enyx/oe/hwstrat/demo/Helper.hpp>

namespace demo = enyx::oe::hwstrat::demo;
namespace data_format = enyx::utils::data_format;

const std::string stream_name = "user0";
std::atomic_bool stop{false};

void signal_handler(int) {
    stop = true;
}

void usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name  << " accelerator_id\n"
        << "Read data sent from the FPGA to the trigger command bus on a standalone test firmware\n";
}


void printer(const std::uint8_t * data, std::uint32_t size) {
    std::cout << data_format::toHexString({data, size}) << "\n";
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