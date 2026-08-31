// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <pxl/algorithm/detail/convert.hpp>

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace pxl {

template
<   typename InputIt
,   typename OutputIt
,   std::floating_point T = float
>
inline void convert
(   InputIt first
,   InputIt last
,   OutputIt out
,   T gamma
)
{   using in_type  = typename std::iterator_traits<InputIt>::value_type;
    using out_type = typename std::iterator_traits<OutputIt>::value_type;
    if constexpr (std::is_same_v<in_type, out_type>)
        std::copy(first, last, out);
    else
    {   if (first == last)
            return;
        auto const count
        =   std::distance(first, last)
        *   static_cast<typename std::iterator_traits<InputIt>::difference_type>
            (   in_type::max_size()
            );
        detail::convert_impl
        (   first->data()
        ,   first->data() + count
        ,   out->data()
        ,   gamma
        ,   std::is_floating_point<typename in_type::channel_type>()
        ,   std::is_floating_point<typename out_type::channel_type>()
        );
    }
}

template
<   std::ranges::random_access_range InputImage
,   std::ranges::random_access_range OutputImage
,   std::floating_point T = float
>
// requires Image<InputImage> && Image<OutputImage>
inline void convert
(   InputImage const& in
,   OutputImage& out
,   T gamma = T(2.2)
)
{   static_assert
    (   InputImage::channels() == OutputImage::channels()
    ,   "Input and output images must have the same number of channels."
    );
    convert(std::begin(in), std::end(in), std::begin(out), gamma);
}

}