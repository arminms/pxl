// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <pxl/algorithm/detail/convert.hpp>

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
        detail::convert_impl
        (   first
        ,   last
        ,   out
        ,   gamma
        ,   std::is_floating_point<typename in_type::channel_type>()
        ,   std::is_floating_point<typename out_type::channel_type>()
        );
}

template
<   std::ranges::contiguous_range InputImage
,   std::ranges::contiguous_range OutputImage
,   std::floating_point T = float
>
requires Image<InputImage> && Image<OutputImage>
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