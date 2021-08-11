#pragma once

#include <stdint.h>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

const uint8_t APPLICATION_VERSION = 1;

class Handler;

struct AlgorithmDispatcher
{
    AlgorithmDispatcher(Handler & handler) : handler_(handler) {}

    void operator()(const uint8_t * data, uint32_t size) ;

    Handler & handler_;
};

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
