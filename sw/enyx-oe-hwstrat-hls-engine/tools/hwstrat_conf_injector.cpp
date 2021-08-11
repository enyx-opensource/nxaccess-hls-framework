#include <iostream>
#include <stdexcept>

#include "injector_helper.hpp"

using namespace enyx::tools;

const std::string stream_name = "user0";

void usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name  << " accelerator_id "  << " [config file] ... \n"
        << "Inject into a test HLS firmware the hardware strategy configuration \n";
}


int main(int argc, char ** argv) {

    if (argc <= 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const std::size_t accelerator_id = std::atoi(argv[1]);
    const Buffers to_inject = parse_simu_files(&argv[2], &argv[argc]);

    try {
       inject(accelerator_id, stream_name, to_inject);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception caught: " << e.what() << "\n";
        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}
