// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#ifdef __CLING__

#include <array>
#include <cstdio>
#include <cmath>
#include <filesystem>

#include <pxl/image.hpp>

#include <xcpp/xdisplay.hpp>
#include <xtl/xbase64.hpp>
#include <nlohmann/json.hpp>
#include <xwidgets/xcolor_picker.hpp>
#include <xwidgets/ximage.hpp>

namespace pxl::detail {

template <typename T, std::size_t C>
std::array<char, 8> color_to_hex // float -> hex string
(   const generic_color<T, C>& color
,   std::true_type
)
{   double const out_max = static_cast<double>(255);
    std::array<char, 8> hex_value;
    std::snprintf
    (   hex_value.data()
    ,   hex_value.size()
    ,   "#%02X%02X%02X"
    //  use the defualt gamma correction of 2.2, which is equivalent to raising the value to the power of 1/2.2 = 0.454545
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(std::pow(static_cast<double>(color[0]), 0.454545) * out_max, 0.0, out_max)))
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(std::pow(static_cast<double>(color[1]), 0.454545) * out_max, 0.0, out_max)))
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(std::pow(static_cast<double>(color[2]), 0.454545) * out_max, 0.0, out_max)))
    );
    return hex_value;
}

template <typename T, std::size_t C>
std::array<char, 8> color_to_hex // int -> hex string
(   const generic_color<T, C>& color
,   std::false_type
)
{   double const in_max
    =   static_cast<double>(std::numeric_limits<T>::max());
    double const out_max = static_cast<double>(255);
    double const scale = out_max / in_max;
    std::array<char, 8> hex_value;
    std::snprintf
    (   hex_value.data()
    ,   hex_value.size()
    ,   "#%02X%02X%02X"
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(static_cast<double>(color[0]) * scale, 0.0, out_max)))
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(static_cast<double>(color[1]) * scale, 0.0, out_max)))
    ,   static_cast<std::uint8_t>(std::llround(std::clamp(static_cast<double>(color[2]) * scale, 0.0, out_max)))
    );
    return hex_value;
}

} // end pxl::detail namespace

#endif // __CLING__
