// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Armin Sobhani
//
#pragma once

#include <cstdint>
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

constexpr uint8_t hexchar_to_int(char ch)
{   if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return 0;
}

}   // end pxl::detail namespace

// -- string literal operator --------------------------------------------------

template<char... Digits>
struct hexcolor_to_uint32;

template<char D, char... Digits>
struct hexcolor_to_uint32<D, Digits...>
{   static_assert
    (   (D >= '0' && D <= '9')
    ||  (D >= 'A' && D <= 'F')
    ||  (D >= 'a' && D <= 'f')
    ||   D == 'x'
    ||   D == 'X'
    ,   "Wrong hex RGBA value!"
    );
    static uint32_t const value
    =   pxl::detail::hexchar_to_int(D)
    *   (1ULL << 4 * sizeof...(Digits))
    +   hexcolor_to_uint32<Digits...>::value;
};

template<char D>
struct hexcolor_to_uint32<D>
{   static_assert
    (   (D >= '0' && D <= '9')
    ||  (D >= 'A' && D <= 'F')
    ||  (D >= 'a' && D <= 'f')
    ||   D == 'x'
    ||   D == 'X'
    ,   "Wrong hex RGBA value!"
    );
    static uint32_t const value = pxl::detail::hexchar_to_int(D);
};

template<char... Digits>
constexpr uint32_t operator"" _rgb()
{   static_assert(8 == sizeof...(Digits), "Hex RGB must have 6 digits!");
    return hexcolor_to_uint32<Digits...>::value << 8 | 0xFF;
}

template<char... Digits>
constexpr uint32_t operator"" _rgba()
{   static_assert(10 == sizeof...(Digits), "Hex RGBA must have 8 digits!");
    return hexcolor_to_uint32<Digits...>::value;
}
