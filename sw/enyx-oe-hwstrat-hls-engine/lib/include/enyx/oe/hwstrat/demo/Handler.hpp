/** @file
 *  @brief Contains Handler class, related functions and documentation.
 *  @author David Keller <david.keller@enyx.fr>
 *  @date 2018
 */

#pragma once

#include <system_error>
#include <enyx/oe/hwstrat/demo/Protocol.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {


/**
 *  @brief Represents the base class you must inherit from in order to 
 *         receive events from the hardware strategy .
 */
class Handler {
protected:
    /**
     *  @brief Protected destructor to force deletion
     *  from derived class.
     *
     *  @note That means ownership of the handler is never passed to
     *  the library.
     */
   virtual ~Handler() = default;

public:
    /** @defgroup HandlerOverrides Algorithm output handler methods to override
     *  @brief Theses method are used to get status from the algorithm
     *  @{
     */
    /**
     *  @brief Called upon reception of an acknowledgement of a configuration
     *  being applied.
     *
     *  @param ack The configuration being acknowledge.
     */
    virtual void on(const InstrumentConfigurationAckMessage& ack) = 0;

    /**
     *  @brief Called upon reception of an acknowledgement of a tick to cancel trigger
     *         being fired.
     *
     *  @param notif Information related to the trigger.
     */
    virtual void on(const TickToCancelNotificationMessage& notif) = 0;


    /**
     *  @brief Called upon reception of an acknowledgement of a tick to trade trigger
     *         being fired.
     *
     *  @param notif Information related to the trigger.
     */
    virtual void on(const TickToTradeNotificationMessage& notif) = 0;

    /// @}

    /**
     *  @brief Called upon an error.
     *  @param error The Error Code.
     */
    virtual void onError(const std::error_code& error) = 0;
};

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx