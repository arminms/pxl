// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <memory>
#include <type_traits>

namespace pxl
{

template <typename T>
struct no_init_allocator : std::allocator<T>
{   template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {   // Do nothing for default-initialization
        if constexpr (sizeof...(Args) > 0)
        {   ::new((void*)p) U(std::forward<Args>(args)...);
        }
    }
};

}  // namespace pxl
