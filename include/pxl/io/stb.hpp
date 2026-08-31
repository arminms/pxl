// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <string>
#include <string_view>

#define STB_IMAGE_IMPLEMENTATION
#include <pxl/io/detail/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <pxl/io/detail/stb_image_write.h>

#include <pxl/concepts.hpp>
#include <pxl/image_view.hpp>
#include <pxl/algorithm/convert.hpp>

namespace pxl::io {

namespace in {

template <Image ImageType>
struct stb
{   stb(float gamma = 2.2f)
    {   if constexpr (std::is_floating_point<typename ImageType::channel_type>())
            stbi_ldr_to_hdr_gamma(gamma);
        else
            stbi_hdr_to_ldr_gamma(gamma);
    }
    ImageType operator() (std::string_view filename)
    {   int w, h, n;
        std::string path(filename);
        if constexpr (std::is_floating_point<typename ImageType::channel_type>())
        {   float* data = stbi_loadf
            (   path.c_str()
            ,   &w
            ,   &h
            ,   &n
            ,   ImageType::channels()
            );
            if (nullptr == data)
                throw std::runtime_error
                    ("pxl::stb: could not open file -> " + path);
            pxl::image_view<float, ImageType::channels()> v
            (   reinterpret_cast<pxl::generic_color<float, ImageType::channels()>*>(data)
            ,   0, 0, w, h, w
            );
            ImageType r(w, h);
            pxl::convert(v.begin(), v.end(), r.begin(), 2.2f);
            stbi_image_free(data);
            return r;
        }
        else
        {   uint8_t* data = stbi_load
            (   path.c_str()
            ,   &w
            ,   &h
            ,   &n
            ,   ImageType::channels()
            );
            if (nullptr == data)
                throw std::runtime_error
                    ("pxl::stb: could not open file -> " + path);
            pxl::image_view<uint8_t, ImageType::channels()> v
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(data)
            ,   0, 0, w, h, w
            );
            ImageType r(w, h);
            pxl::convert(v.begin(), v.end(), r.begin(), 2.2f);
            stbi_image_free(data);
            return r;
        }
    }
};

} // end in namespace

} // end pxl::io namespace
