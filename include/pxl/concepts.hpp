// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Armin Sobhani
//
#pragma once

#include <type_traits>
#include <concepts>
#include <vector>
#include <iterator>
#include <stdexcept>

namespace pxl {

template<typename T>
concept has_value_type = requires
{   typename T::value_type;
};

template<typename T>
concept has_size_type = requires
{   typename T::size_type;
};

template<typename T>
concept has_container_type = requires
{   typename T::container_type;
};

template<typename T>
concept has_channel_type = requires
{   typename T::channel_type;
};

template<typename T>
concept has_pixel_type = requires
{   typename T::pixel_type;
};

template<typename T>
concept Pixel
=   has_channel_type<T>
&&  has_value_type<T>
&&  has_size_type<T>
;

template<typename T>
concept Image
=   has_container_type<T>
&&  has_pixel_type<T>
&&  has_channel_type<T>
&&  has_value_type<T>
&&  has_size_type<T>
;

} // end pxl namespace