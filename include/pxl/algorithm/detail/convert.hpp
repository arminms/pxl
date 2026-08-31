// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <pxl/concepts.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>

namespace pxl::detail {

// All four overloads below operate on ranges of *channel* scalars (not
// pixel/color types) - [first, last) and out must be iterators over the
// individual `channel_type`s of the pixels being converted. Scaling is
// always based on the compile-time representable range of the channel
// types involved (numeric_limits::max()), never on the actual data range.

template
<   typename InputIt
,   typename OutputIt
,   std::floating_point T = float
>
inline void convert_impl // fp -> fp
(   InputIt first
,   InputIt last
,   OutputIt out
,   T gamma
,   std::true_type
,   std::true_type
)
{   using out_type = typename std::iterator_traits<OutputIt>::value_type;
    while (first != last)
        *out++ = static_cast<out_type>(*first++);
}
template
<   typename InputIt
,   typename OutputIt
,   std::floating_point T = float
>
inline void convert_impl // fp -> int
(   InputIt first
,   InputIt last
,   OutputIt out
,   T gamma
,   std::true_type
,   std::false_type
)
{   using out_type = typename std::iterator_traits<OutputIt>::value_type;
    double const out_max
    =   static_cast<double>(std::numeric_limits<out_type>::max());
    double const inv_gamma = 1.0 / static_cast<double>(gamma);
    while (first != last)
    {   double value
        =   std::pow(static_cast<double>(*first++), inv_gamma) * out_max;
        value = std::clamp(value, 0.0, out_max);
        *out++ = static_cast<out_type>(std::llround(value));
    }
}
template
<   typename InputIt
,   typename OutputIt
,   std::floating_point T = float
>
inline void convert_impl // int -> fp
(   InputIt first
,   InputIt last
,   OutputIt out
,   T gamma
,   std::false_type
,   std::true_type
)
{   using in_type  = typename std::iterator_traits<InputIt>::value_type;
    using out_type = typename std::iterator_traits<OutputIt>::value_type;
    double const in_max
    =   static_cast<double>(std::numeric_limits<in_type>::max());
    double const gamma_d = static_cast<double>(gamma);
    while (first != last)
        *out++ = static_cast<out_type>
        (   std::pow(static_cast<double>(*first++) / in_max, gamma_d)
        );
}
template
<   typename InputIt
,   typename OutputIt
,   std::floating_point T = float
>
inline void convert_impl // int -> int
(   InputIt first
,   InputIt last
,   OutputIt out
,   T gamma
,   std::false_type
,   std::false_type
)
{   using in_type  = typename std::iterator_traits<InputIt>::value_type;
    using out_type = typename std::iterator_traits<OutputIt>::value_type;
    double const in_max
    =   static_cast<double>(std::numeric_limits<in_type>::max());
    double const out_max
    =   static_cast<double>(std::numeric_limits<out_type>::max());
    double const scale = out_max / in_max;
    while (first != last)
    {   double const value
        =   std::clamp(static_cast<double>(*first++) * scale, 0.0, out_max);
        *out++ = static_cast<out_type>(std::llround(value));
    }
}

} // end pxl::detail namespace