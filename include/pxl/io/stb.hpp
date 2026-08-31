// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <pxl/io/detail/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <pxl/io/detail/stb_image_write.h>

#include <pxl/concepts.hpp>
#include <pxl/image_view.hpp>
#include <pxl/memory.hpp>
#include <pxl/algorithm/convert.hpp>

namespace pxl::io {

namespace in {

template <typename ImageType>
struct stb
{   stb(float gamma = 2.2f)
    {   if constexpr (std::is_floating_point<typename ImageType::channel_type>())
            stbi_ldr_to_hdr_gamma(gamma);
        else
            stbi_hdr_to_ldr_gamma(gamma);
    }
    ImageType operator() (std::string_view filename)
    {   std::string path(filename); // ensures a null-terminated buffer for the C API
        int w, h, n;
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
            pxl::convert(v, r);
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
            pxl::convert(v, r);
            stbi_image_free(data);
            return r;
        }
    }
};

} // end in namespace

namespace out {

// writes a stbi_write_* buffer into the std::stringstream passed as context
inline void sstream_write_func(void* context, void* data, int size)
{   static_cast<std::stringstream*>(context)->write
    (   static_cast<const char*>(data)
    ,   size
    );
}

struct png
{   template <Image ImageType>
    int operator() (std::string_view filename, const ImageType& image)
    {   std::string path(filename); // ensures a null-terminated buffer for the C API
        if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_png
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            ,   0
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_png
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            ,   0
            );
        }
    }
    template <Image ImageType>
    int operator() (std::stringstream& ss, const ImageType& image)
    {   if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_png_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            ,   0
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_png_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            ,   0
            );
        }
    }
};
struct bmp
{   template <Image ImageType>
    int operator() (std::string_view filename, const ImageType& image)
    {   std::string path(filename); // ensures a null-terminated buffer for the C API
        if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_bmp
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_bmp
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            );
        }
    }
    template <Image ImageType>
    int operator() (std::stringstream& ss, const ImageType& image)
    {   if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_bmp_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_bmp_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            );
        }
    }
};
struct jpg
{   template <Image ImageType>
    int operator() (std::string_view filename, const ImageType& image)
    {   std::string path(filename); // ensures a null-terminated buffer for the C API
        if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_jpg
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            ,   90
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_jpg
            (   path.c_str()
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            ,   90
            );
        }
    }
    template <Image ImageType>
    int operator() (std::stringstream& ss, const ImageType& image)
    {   if constexpr (std::is_same_v<typename ImageType::value_type, uint8_t>)
            return stbi_write_jpg_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   image.data()
            ,   90
            );
        else
        {   std::vector<uint8_t, no_init_allocator<uint8_t>> v
            (   image.width()
            *   image.height()
            *   ImageType::channels()
            );
            pxl::image_view<uint8_t, ImageType::channels()> view
            (   reinterpret_cast<pxl::generic_color<uint8_t, ImageType::channels()>*>(v.data())
            ,   0, 0, image.width(), image.height(), image.width()
            );
            pxl::convert(image, view);
            return stbi_write_jpg_to_func
            (   sstream_write_func
            ,   &ss
            ,   static_cast<int>(image.width())
            ,   static_cast<int>(image.height())
            ,   ImageType::channels()
            ,   v.data()
            ,   90
            );
        }
    }
};

/// @brief Default writer that picks png/bmp/jpg based on the filename
/// extension (defaults to png for the stringstream overload and for
/// unrecognized/missing extensions).
template <typename ImageType>
struct stb
{   int operator() (std::string_view filename, const ImageType& image)
    {   if (filename.ends_with(".bmp"))
            return bmp{}(filename, image);
        if (filename.ends_with(".jpg") || filename.ends_with(".jpeg"))
            return jpg{}(filename, image);
        return png{}(filename, image);
    }
    int operator() (std::stringstream& ss, const ImageType& image)
    {   return png{}(ss, image);
    }
};

} // end out namespace

} // end pxl::io namespace

