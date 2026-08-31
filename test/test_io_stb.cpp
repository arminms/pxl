// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#include <pxl/image.hpp>
#include <pxl/io/stb.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::WithinAbs;
using color_type = pxl::color<uint8_t, 3>;
using image_type = pxl::image<uint8_t, 3>;
using image4_type = pxl::image<uint8_t, 4>;
using image_float_type = pxl::image<float, 3>;

namespace
{

// a temp file path unique to the calling test case
std::filesystem::path temp_path(std::string_view name)
{   return std::filesystem::temp_directory_path() / name;
}

} // end anonymous namespace

// -- io::in::stb ----------------------------------------------------------

TEST_CASE
(   "io::in::stb reads back an 8-bit PNG with matching pixel values"
,   "[io][stb][in]"
)
{   auto path = temp_path("pxl_test_in_stb_rgb.png");
    std::vector<uint8_t> pixels
    {   255,   0,   0,     0, 255,   0
    ,     0,   0, 255,   255, 255,   0
    };
    REQUIRE
    (   stbi_write_png
        (   path.string().c_str()
        ,   2, 2, 3
        ,   pixels.data()
        ,   2 * 3
        )   != 0
    );

    pxl::io::in::stb<image_type> reader;
    image_type img = reader(path.string());
    std::filesystem::remove(path);

    REQUIRE(img.width() == 2);
    REQUIRE(img.height() == 2);
    REQUIRE(img[0][0] == color_type{255, 0, 0});
    REQUIRE(img[0][1] == color_type{0, 255, 0});
    REQUIRE(img[1][0] == color_type{0, 0, 255});
    REQUIRE(img[1][1] == color_type{255, 255, 0});
}
TEST_CASE
(   "io::in::stb requests the channel count of the target image type"
,   "[io][stb][in]"
)
{   auto path = temp_path("pxl_test_in_stb_rgba.png");
    std::vector<uint8_t> pixels
    {   10, 20, 30
    ,   40, 50, 60
    };
    REQUIRE
    (   stbi_write_png
        (   path.string().c_str()
        ,   2, 1, 3
        ,   pixels.data()
        ,   2 * 3
        )   != 0
    );

    pxl::io::in::stb<image4_type> reader;
    image4_type img = reader(path.string());
    std::filesystem::remove(path);

    REQUIRE(img.width() == 2);
    REQUIRE(img.height() == 1);
    // stbi_load synthesizes an opaque alpha channel when up-converting
    REQUIRE(img[0][0] == pxl::color<uint8_t, 4>{10, 20, 30, 255});
    REQUIRE(img[0][1] == pxl::color<uint8_t, 4>{40, 50, 60, 255});
}
TEST_CASE
(   "io::in::stb reads back a floating-point HDR file"
,   "[io][stb][in]"
)
{   auto path = temp_path("pxl_test_in_stb.hdr");
    std::vector<float> pixels
    {   0.1f, 0.2f, 0.3f
    ,   0.4f, 0.5f, 0.6f
    };
    REQUIRE
    (   stbi_write_hdr
        (   path.string().c_str()
        ,   2, 1, 3
        ,   pixels.data()
        )   != 0
    );

    pxl::io::in::stb<image_float_type> reader;
    image_float_type img = reader(path.string());
    std::filesystem::remove(path);

    REQUIRE(img.width() == 2);
    REQUIRE(img.height() == 1);
    for (std::size_t c = 0; c < 3; ++c)
    {   REQUIRE_THAT(img[0][0][c], WithinAbs(pixels[c], 0.01));
        REQUIRE_THAT(img[0][1][c], WithinAbs(pixels[3 + c], 0.01));
    }
}
TEST_CASE
(   "io::in::stb throws std::runtime_error for a missing file"
,   "[io][stb][in]"
)
{   pxl::io::in::stb<image_type> reader;
    REQUIRE_THROWS_AS
    (   reader("pxl_test_in_stb_does_not_exist.png")
    ,   std::runtime_error
    );
}

// -- io::out::png/bmp/jpg ---------------------------------------------------

TEST_CASE
(   "io::out::png writes an 8-bit image that reads back exactly"
,   "[io][stb][out]"
)
{   auto path = temp_path("pxl_test_out_png_u8.png");
    image_type img(2, 2, color_type{0, 0, 0});
    img[0][0] = color_type{255, 0, 0};
    img[0][1] = color_type{0, 255, 0};
    img[1][0] = color_type{0, 0, 255};
    img[1][1] = color_type{255, 255, 0};

    pxl::io::out::png writer;
    REQUIRE(writer(path.string(), img) != 0);

    int w, h, n;
    uint8_t* data = stbi_load(path.string().c_str(), &w, &h, &n, 3);
    std::filesystem::remove(path);
    REQUIRE(data != nullptr);
    REQUIRE(w == 2);
    REQUIRE(h == 2);
    REQUIRE(data[0] == 255); REQUIRE(data[1] == 0);   REQUIRE(data[2] == 0);
    REQUIRE(data[3] == 0);   REQUIRE(data[4] == 255); REQUIRE(data[5] == 0);
    stbi_image_free(data);
}
TEST_CASE
(   "io::out::png converts non-uint8_t channels before writing"
,   "[io][stb][out]"
)
{   auto path = temp_path("pxl_test_out_png_u16.png");
    pxl::image<uint16_t, 3> img(1, 1, pxl::color<uint16_t, 3>{65535, 0, 32896});

    pxl::io::out::png writer;
    REQUIRE(writer(path.string(), img) != 0);

    int w, h, n;
    uint8_t* data = stbi_load(path.string().c_str(), &w, &h, &n, 3);
    std::filesystem::remove(path);
    REQUIRE(data != nullptr);
    REQUIRE(w == 1);
    REQUIRE(h == 1);
    REQUIRE(data[0] == 255); // 65535 rescaled to uint8_t max
    REQUIRE(data[1] == 0);
    REQUIRE(data[2] == 128); // 32896 rescaled to uint8_t
    stbi_image_free(data);
}
TEST_CASE
(   "io::out::png writes to a stringstream matching the file output"
,   "[io][stb][out]"
)
{   image_type img(2, 1, color_type{1, 2, 3});
    img[0][1] = color_type{4, 5, 6};

    pxl::io::out::png writer;
    std::stringstream ss;
    REQUIRE(writer(ss, img) != 0);

    std::string bytes = ss.str();
    int w, h, n;
    uint8_t* data = stbi_load_from_memory
    (   reinterpret_cast<const uint8_t*>(bytes.data())
    ,   static_cast<int>(bytes.size())
    ,   &w, &h, &n, 3
    );
    REQUIRE(data != nullptr);
    REQUIRE(w == 2);
    REQUIRE(h == 1);
    REQUIRE(data[0] == 1); REQUIRE(data[1] == 2); REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4); REQUIRE(data[4] == 5); REQUIRE(data[5] == 6);
    stbi_image_free(data);
}
TEST_CASE
(   "io::out::bmp writes a readable file and stringstream"
,   "[io][stb][out]"
)
{   auto path = temp_path("pxl_test_out.bmp");
    image_type img(2, 2, color_type{9, 8, 7});

    pxl::io::out::bmp writer;
    REQUIRE(writer(path.string(), img) != 0);

    int w, h, n;
    uint8_t* data = stbi_load(path.string().c_str(), &w, &h, &n, 3);
    std::filesystem::remove(path);
    REQUIRE(data != nullptr);
    REQUIRE(w == 2);
    REQUIRE(h == 2);
    REQUIRE(data[0] == 9);
    stbi_image_free(data);

    std::stringstream ss;
    REQUIRE(writer(ss, img) != 0);
    std::string bytes = ss.str();
    data = stbi_load_from_memory
    (   reinterpret_cast<const uint8_t*>(bytes.data())
    ,   static_cast<int>(bytes.size())
    ,   &w, &h, &n, 3
    );
    REQUIRE(data != nullptr);
    stbi_image_free(data);
}
TEST_CASE
(   "io::out::jpg writes a readable (lossy) file and stringstream"
,   "[io][stb][out]"
)
{   auto path = temp_path("pxl_test_out.jpg");
    image_type img(4, 4, color_type{200, 100, 50});

    pxl::io::out::jpg writer;
    REQUIRE(writer(path.string(), img) != 0);

    int w, h, n;
    uint8_t* data = stbi_load(path.string().c_str(), &w, &h, &n, 3);
    std::filesystem::remove(path);
    REQUIRE(data != nullptr);
    REQUIRE(w == 4);
    REQUIRE(h == 4);
    stbi_image_free(data);

    std::stringstream ss;
    REQUIRE(writer(ss, img) != 0);
    std::string bytes = ss.str();
    data = stbi_load_from_memory
    (   reinterpret_cast<const uint8_t*>(bytes.data())
    ,   static_cast<int>(bytes.size())
    ,   &w, &h, &n, 3
    );
    REQUIRE(data != nullptr);
    stbi_image_free(data);
}

// -- io::out::stb (extension-based dispatcher) ------------------------------

TEST_CASE
(   "io::out::stb dispatches by filename extension"
,   "[io][stb][out]"
)
{   image_type img(2, 2, color_type{1, 2, 3});
    pxl::io::out::stb<image_type> writer;

    auto png_path = temp_path("pxl_test_out_stb.png");
    auto bmp_path = temp_path("pxl_test_out_stb.bmp");
    auto jpg_path = temp_path("pxl_test_out_stb.jpg");
    auto unknown_path = temp_path("pxl_test_out_stb.unknown");

    REQUIRE(writer(png_path.string(), img) != 0);
    REQUIRE(writer(bmp_path.string(), img) != 0);
    REQUIRE(writer(jpg_path.string(), img) != 0);
    REQUIRE(writer(unknown_path.string(), img) != 0); // falls back to png

    for (auto p : {png_path, bmp_path, jpg_path, unknown_path})
    {   int w, h, n;
        uint8_t* data = stbi_load(p.string().c_str(), &w, &h, &n, 3);
        std::filesystem::remove(p);
        REQUIRE(data != nullptr);
        REQUIRE(w == 2);
        REQUIRE(h == 2);
        stbi_image_free(data);
    }
}
TEST_CASE
(   "io::out::stb writes a stringstream as png"
,   "[io][stb][out]"
)
{   image_type img(1, 1, color_type{42, 43, 44});
    pxl::io::out::stb<image_type> writer;

    std::stringstream ss;
    REQUIRE(writer(ss, img) != 0);

    std::string bytes = ss.str();
    int w, h, n;
    uint8_t* data = stbi_load_from_memory
    (   reinterpret_cast<const uint8_t*>(bytes.data())
    ,   static_cast<int>(bytes.size())
    ,   &w, &h, &n, 3
    );
    REQUIRE(data != nullptr);
    REQUIRE(data[0] == 42); REQUIRE(data[1] == 43); REQUIRE(data[2] == 44);
    stbi_image_free(data);
}
