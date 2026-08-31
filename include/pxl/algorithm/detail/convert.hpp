// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <pxl/concepts.hpp>

namespace pxl::detail {

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
{   using in_type  = typename std::iterator_traits<InputIt>::value_type;
    using out_type = typename std::iterator_traits<OutputIt>::value_type;
    in_type one_over_gamma
    =   static_cast<in_type>(1)
    /   static_cast<in_type>(gamma);
    auto minmax = std::minmax_element(first, last);
    auto min = *minmax.first;
    auto max = *minmax.second;
    // float one_over_range = 1.0f / (max - min);
    in_type out_max = std::min
    (   static_cast<in_type>(std::numeric_limits<out_type>::max())
    ,   max / min
    );
    while (first != last)
        *out++ = static_cast<out_type>
        (   std::max
            (   static_cast<in_type>(0)
            ,   std::min
                (   out_max
                ,   std::pow
                    // (   (*first++ - min) * one_over_range
                    (   *first++
                    ,   one_over_gamma
                    )
                    *   out_max
                    +   std::numeric_limits<in_type>::round_error()
                )
            )
        );
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
    T scale{};
    if constexpr (std::numeric_limits<in_type>::max() > 255)
    {   auto max = std::max_element(first, last);
        scale
        =   static_cast<T>(1)
        /   std::min(*max * 10, std::numeric_limits<in_type>::max());
    }
    else
        scale
        =   static_cast<T>(1)
        /   static_cast<T>(std::numeric_limits<in_type>::max());
    while (first != last)
        *out++ = static_cast<out_type>(std::pow(*first++ * scale, gamma));
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
    T scale{};
    if constexpr
    (   std::numeric_limits<in_type>::max()
    >   std::numeric_limits<out_type>::max()
    )
    {   auto max  = std::max_element(first, last);
        scale
        =   static_cast<T>(std::numeric_limits<out_type>::max())
        /   std::min(*max * 10, std::numeric_limits<in_type>::max());
        while (first != last)
            *out++ = static_cast<out_type>
            (   std::pow(*first++ * scale, gamma)
            +   std::numeric_limits<T>::round_error()
            );
    }
    else
    {   scale
        =   static_cast<T>(std::numeric_limits<out_type>::max())
        *   100
        /   static_cast<T>(std::numeric_limits<in_type>::max());
        while (first != last)
            *out++ = static_cast<out_type>
            (   *first++
            *   scale
            /   100 // ?
            +   std::numeric_limits<in_type>::round_error()
            );
    }
}

} // end pxl::detail namespace