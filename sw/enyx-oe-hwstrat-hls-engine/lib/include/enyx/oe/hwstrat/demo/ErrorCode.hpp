/** @file
 *  @brief Contains ErrorCode enum, related functions and documentation.
 *  @author David Keller <david.keller@enyx.fr>
 *  @date 2018
 */

#pragma once

#include <system_error>

namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {


/// Enums to represents
enum ErrorCodeEnum {
    UNIMPLEMENTED = 1,
    CORRUPTED_APPLICATION_HEADER,
    UNKNOWN_ALGORITHM_MESSAGE,
};

/**
 *  Retrieve the client library error category.
 *
 *  @return A constant reference to the category.
 */
const std::error_category &
errorCategory();

/**
 *  Create a standard error condition instance from an ErrorCode enum.
 *
 *  @param index The error index.
 *  @return A standard error code instance bound with the client category.
 */
inline std::error_condition
make_error_condition(ErrorCodeEnum index) {
    return std::error_condition(static_cast<int>(index), errorCategory());
}

/**
 *  Create a standard error code instance from an ErrorCode enum.
 *
 *  @param index The error index.
 *  @return A standard error code instance bound with the client category.
 */
inline std::error_code
make_error_code(ErrorCodeEnum index) {
    return std::error_code(static_cast<int>(index), errorCategory());
}


} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx

/// @cond
namespace std {

template<>
struct is_error_condition_enum<enyx::oe::hwstrat::demo::ErrorCodeEnum> : std::true_type { };


} // namespace std
/// @endcond