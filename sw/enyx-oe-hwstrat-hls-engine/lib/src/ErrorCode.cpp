#include <enyx/oe/hwstrat/demo/ErrorCode.hpp>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

namespace {

struct CategoryImpl
          : std::error_category
{
    CategoryImpl()
        : std::error_category() {
    }

    virtual const char*
    name() const noexcept {
        return "oe-hwstrat-hls";
    }

    virtual std::string
    message(int condition) const {
        switch (condition) {
            case UNIMPLEMENTED: return "Unimplemented";
            case CORRUPTED_APPLICATION_HEADER: return "Corrupted application header";
            case UNKNOWN_ALGORITHM_MESSAGE: return "Unknown algorithm message";
            default: return "Unknown error";
        }
    }
};

} // namespace anonymous



const std::error_category &
errorCategory() {
    const static CategoryImpl CATEGORY;
    return CATEGORY;
}
} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
