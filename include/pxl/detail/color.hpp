// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Armin Sobhani
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace pxl::detail {

template <std::size_t ... Is>
constexpr auto indexSequenceReverse
(   std::index_sequence<Is...> const &)
->  decltype( std::index_sequence<sizeof...(Is)-1U-Is...>{}
);

template <std::size_t N>
using makeIndexSequenceReverse
=   decltype(indexSequenceReverse(std::make_index_sequence<N>{}));

template<std::size_t N>
struct num { static const constexpr auto value = N; };

template <class F, std::size_t... Is>
constexpr void for_(F func, std::index_sequence<Is...>)
{   using expander = int[];
    (void)expander{0, ((void)func(num<Is>{}), 0)...};
}

template <std::size_t N, typename F>
constexpr void for_(F func)
{   for_(func, makeIndexSequenceReverse<N>());
}

template<std::size_t N, typename T>
struct uint32_to_array
{};

template<typename T>
struct uint32_to_array<1, T> // 8-bits per channels
{   constexpr static T convert(uint32_t rgba, std::false_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c) = rgba & 0xFF;
                rgba >>= 8;
            }
        );
        return c;
    }
    constexpr static T convert(uint32_t rgba, std::true_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c)
            =   (rgba & 0xFF)
            *   (typename T::value_type)3.921568e-3;
                rgba >>= 8;
            }
        );
        return c;
    }
};

template<typename T>
struct uint32_to_array<2, T> // 16-bits per channels
{   constexpr static T convert(uint32_t rgba, std::false_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c) = (rgba & 0xFF) * 0xFF;
                rgba >>= 8;
            }
        );
        return c;
    }
    constexpr static T convert(uint32_t rgba, std::true_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c)
            =   (rgba & 0xFF)
            *   (typename T::value_type)3.921568e-3;
                rgba >>= 8;
            }
        );
        return c;
    }
};

template<typename T>
struct uint32_to_array<4, T> // 32-bits per channels
{   constexpr static T convert(uint32_t rgba, std::false_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c) = (rgba & 0xFF) * 0xFFFF;
                rgba >>= 8;
            }
        );
        return c;
    }
    constexpr static T convert(uint32_t rgba, std::true_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c)
            =   (rgba & 0xFF)
            *   (typename T::value_type)3.921568e-3;
                rgba >>= 8;
            }
        );
        return c;
    }
};

template<typename T>
struct uint32_to_array<8, T> // 64-bits per channels
{   constexpr static T convert(uint32_t rgba, std::false_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c) = (rgba & 0xFF) * 0xFFFFFF;
                rgba >>= 8;
            }
        );
        return c;
    }
    constexpr static T convert(uint32_t rgba, std::true_type)
    {   T c{0, 0, 0, 0};
        for_<c.size()>
        (   [&] (auto i)
            {   std::get<i.value>(c)
            =   (rgba & 0xFF)
            *   (typename T::value_type)3.921568627451e-3;
                rgba >>= 8;
            }
        );
        return c;
    }
};

constexpr uint8_t hex_digit_to_int(char c)
{   if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    throw std::invalid_argument("Invalid hex character");
}

constexpr uint32_t operator""_rgba(const char* str, std::size_t len)
{   // Valid lengths with '#': 7 for #RRGGBB, 9 for #RRGGBBAA
    if (str[0] != '#' || (len != 7 && len != 9))
        throw std::invalid_argument("RGBA literal must be in '#RRGGBB' or '#RRGGBBAA' format");

    uint32_t r = (hex_digit_to_int(str[1]) << 4) | hex_digit_to_int(str[2]);
    uint32_t g = (hex_digit_to_int(str[3]) << 4) | hex_digit_to_int(str[4]);
    uint32_t b = (hex_digit_to_int(str[5]) << 4) | hex_digit_to_int(str[6]);
    
    // Parse alpha if present; default to fully opaque (0xFF)
    uint32_t a
    =   (len == 9) 
    ?   ((hex_digit_to_int(str[7]) << 4) | hex_digit_to_int(str[8]))
    :   0xFF;

    return (r << 24) | (g << 16) | (b << 8) | a;
}

} // end pxl::detail namespace

// -- string literal operator --------------------------------------------------

namespace pxl::literals {

template <std::size_t N>
struct FixedString
{   char buf[N]{};
    constexpr FixedString(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i) buf[i] = s[i];
    }
};

template <FixedString FS>
constexpr uint32_t operator""_rgba()
{   // Exact length check: '#' + 6 hex digits + null terminator -> size 9
    static_assert
    (   sizeof(FS.buf) >= 8
    &&  sizeof(FS.buf) <= 10
    ,   "RGB literal must be in '#RRGGBB' or '#RRGGBBAA' format"
    );
    static_assert(FS.buf[0] == '#', "RGB literal must start with '#'");
    return pxl::detail::operator""_rgba(FS.buf, sizeof(FS.buf) - 1);
}

} // end pxl::literals namespace