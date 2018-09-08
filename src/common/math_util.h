/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include <utility>

#include "common/common_types.h"

namespace Dynarmic::Common {

/**
 * This function is a workaround for a bug in MSVC 19.12 where fold expressions
 * do not work when the /permissive- flag is enabled.
 */
template<typename T, typename... Ts>
constexpr T Sum(T first, Ts&&... rest) {
    if constexpr (sizeof...(rest) == 0) {
        return first;
    } else {
        return first + Sum(std::forward<Ts>(rest)...);
    }
}

/**
 * Input is a u0.9 fixed point number. Only values in [0.5, 1.0) are valid.
 * Output is a u0.8 fixed point number, with an implied 1 prefixed.
 * i.e.: The output is a value in [1.0, 2.0).
 *
 * @see RecipEstimate() within the ARMv8 architecture reference manual
 *      for a general overview of the requirements of the algorithm.
 */
u8 RecipEstimate(u64 a);

/**
 * Input is a u0.9 fixed point number. Only values in [0.25, 1.0) are valid.
 * Output is a u0.8 fixed point number, with an implied 1 prefixed.
 * i.e.: The output is a value in [1.0, 2.0).
 *
 * @see RecipSqrtEstimate() within the ARMv8 architecture reference manual
 *      for a general overview of the requirements of the algorithm.
 */
u8 RecipSqrtEstimate(u64 a);

} // namespace Dynarmic::Common
