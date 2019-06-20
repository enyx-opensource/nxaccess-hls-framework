/**
 * @example HwStratHlsDemo.cpp
 *          This example connects to the exchange and configures the algorithm.
 */
/// @cond
/// [HeaderInclusion]
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <thread>
#include <future>
#include <unordered_map>

// Include enyx handler.
#include <enyx/utils/log/macros.hpp>
#include <enyx/oe/hwstrat/demo/AlgorithmDriver.hpp>
#include <enyx/oe/hwstrat/demo/Handler.hpp>
#include <enyx/oe/hwstrat/demo/Protocol.hpp>
/// [HeaderInclusion]


namespace oe = enyx::oe;
namespace hwstrat = oe::hwstrat;


/// [ExitHandler]
namespace {

// Log prefix
const char * LogPrefix = "hwstrat_hls_demo";

// Used to signal the process must terminate.
volatile std::atomic<bool> isExitRequested;

// Handler assocaited with SIGINT
void signalExit(int = 0) {
    isExitRequested = true;
}

// Called when dispatching exit because of network error.
void quit(const std::error_code & failure = std::error_code{}) {
    if (failure) {
        LOG_ME(NX_WARNING, "[%s] %s.", LogPrefix, failure.message().c_str());
    }
    signalExit();
}


// Transform type to string
template <typename T>
std::string
toStr(const T& v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}

} // namespace
/// [ExitHandler]

/// [HandlerDefinition]
namespace enyx {
namespace example {



/**
 *  @brief Handler use to receive response messages from the algorithm.
 */
struct OrderHandler : public oe::hwstrat::demo::Handler {
    /**
     *  @brief Called upon reception of an output from the algorithm.
     *
     *  @param error The error from the algorithm.
     */
    virtual void onError(const std::error_code & error) override {
        LOG_ME(NX_WARNING, "[%s] %s.", LogPrefix, error.message().c_str());
        quit(error);
    }

    /**
     *  @brief Called upon reception of an output from the algorithm.
     *
     *  @param event The output of the algorithm.
     */
    virtual void
    on(const hwstrat::demo::InstrumentConfigurationAckMessage & ack) override {
        LOG_ME(NX_INFO, "[%s] ConfAck: %s", LogPrefix, toStr(ack).c_str());
    }

    virtual void
    on(const hwstrat::demo::TickToCancelNotificationMessage& notif) override {
        LOG_ME(NX_INFO, "[%s] TickToCancel: %s", LogPrefix, toStr(notif).c_str());
    }

    virtual void
    on(const hwstrat::demo::TickToTradeNotificationMessage& notif) override {
        LOG_ME(NX_INFO, "[%s] TickToTrade: %s", LogPrefix, toStr(notif).c_str());
    }
};

} // namespace example
} // namespace enyx

/// [HandlerDefinition]
/// [Main]
// Main entry point.
int main(int argc, char* argv[]) {

    // Check arguments.
    if (argc != 2) {
        LOG_ME(NX_WARNING, "[%s] usage: %s BOARD_INDEX",
            LogPrefix, argv[0]);
        return EXIT_FAILURE;
    }

    const std::uint16_t boardIndex = std::atoi(argv[1]);

/// [Main]
    try {
/// [DriverCreation]
        // Instantiate our handler.
        enyx::example::OrderHandler myHandler;

        LOG_ME(NX_INFO, "[%s] Working on board %d", LogPrefix, boardIndex)
        hwstrat::demo::AlgorithmDriver algorithm{myHandler,
                                                 boardIndex};


        // Setup clean exit.
        LOG_ME(NX_INFO, "[%s] Registering signals.", LogPrefix);
        std::signal(SIGINT, &signalExit);
        std::signal(SIGTERM, &signalExit);

        LOG_ME(NX_INFO, "[%s] Send CTRL-C to quit.", LogPrefix);
        /// Start dispatching of algorithm from_fpga.
        while (! isExitRequested) {
            algorithm.poll();
        }
        LOG_ME(NX_INFO, "[%s] Polling stopped", LogPrefix);

/// [DriverCreation]

/// [ErrorChecking]
    } catch (const std::exception & e) {
        LOG_ME(NX_WARNING, "[%s] %s", LogPrefix, e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LOG_ME(NX_WARNING, "[%s] unknown error.", LogPrefix);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
/// [ErrorChecking]

/// @endcond