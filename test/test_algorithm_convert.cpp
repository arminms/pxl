// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#include <pxl/algorithm/convert.hpp>
#include <pxl/color.hpp>
#include <pxl/image.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

using Catch::Matchers::WithinAbs;
using pxl::generic_color;

// -- integer <-> integer -------------------------------------------------------

TEST_CASE
(   "convert rescales uint8_t channels up to uint16_t"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 1>> in
    {   {0}, {128}, {255}
    };
    std::vector<generic_color<uint16_t, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0);
    REQUIRE(out[1][0] == 32896); // round(128 * 65535 / 255)
    REQUIRE(out[2][0] == 65535);
}
TEST_CASE
(   "convert rescales uint16_t channels down to uint8_t"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint16_t, 1>> in
    {   {0}, {32896}, {65535}
    };
    std::vector<generic_color<uint8_t, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0);
    REQUIRE(out[1][0] == 128); // round(32896 * 255 / 65535)
    REQUIRE(out[2][0] == 255);
}
TEST_CASE
(   "convert round-trips uint8_t -> uint16_t -> uint8_t without loss"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 1>> in;
    for (int i = 0; i <= 255; ++i)
        in.push_back({static_cast<uint8_t>(i)});
    std::vector<generic_color<uint16_t, 1>> mid(in.size());
    std::vector<generic_color<uint8_t, 1>> back(in.size());
    pxl::convert(in.begin(), in.end(), mid.begin(), 1.0f);
    pxl::convert(mid.begin(), mid.end(), back.begin(), 1.0f);
    for (std::size_t i = 0; i < in.size(); ++i)
        REQUIRE(back[i][0] == in[i][0]);
}
TEST_CASE
(   "convert rescales uint8_t channels up to uint32_t"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 1>> in{{0}, {255}};
    std::vector<generic_color<uint32_t, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0u);
    REQUIRE(out[1][0] == 4294967295u);
}

// -- integer <-> floating point -------------------------------------------------

TEST_CASE
(   "convert normalizes uint8_t channels to float with gamma 1.0"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 1>> in{{0}, {128}, {255}};
    std::vector<generic_color<float, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE_THAT(out[0][0], WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(out[1][0], WithinAbs(128.0f / 255.0f, 1e-6f));
    REQUIRE_THAT(out[2][0], WithinAbs(1.0f, 1e-6f));
}
TEST_CASE
(   "convert applies gamma when normalizing integer channels to float"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 1>> in{{128}};
    std::vector<generic_color<float, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 2.2f);
    REQUIRE_THAT
    (   out[0][0]
    ,   WithinAbs(std::pow(128.0f / 255.0f, 2.2f), 1e-5f)
    );
}
TEST_CASE
(   "convert denormalizes float channels to uint8_t with gamma 1.0"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<float, 1>> in{{0.0f}, {0.5f}, {1.0f}};
    std::vector<generic_color<uint8_t, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0);
    REQUIRE(out[1][0] == 128); // round(0.5 * 255)
    REQUIRE(out[2][0] == 255);
}
TEST_CASE
(   "convert clamps out-of-range float channels when converting to uint8_t"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<float, 1>> in{{-1.0f}, {2.0f}};
    std::vector<generic_color<uint8_t, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0);
    REQUIRE(out[1][0] == 255);
}

// -- floating point <-> floating point ------------------------------------------

TEST_CASE
(   "convert casts float channels to double without change in value"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<float, 1>> in{{0.25f}};
    std::vector<generic_color<double, 1>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 2.2);
    REQUIRE_THAT(out[0][0], WithinAbs(0.25, 1e-9));
}

// -- same channel type -----------------------------------------------------------

TEST_CASE
(   "convert copies pixels unchanged when input and output pixel types match"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 3>> in{{1, 2, 3}, {4, 5, 6}};
    std::vector<generic_color<uint8_t, 3>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 2.2f);
    REQUIRE(out[0] == in[0]);
    REQUIRE(out[1] == in[1]);
}

// -- multi-channel / image-level convert -----------------------------------------

TEST_CASE
(   "convert rescales every channel of a multi-channel pixel"
,   "[algorithm][convert]"
)
{   std::vector<generic_color<uint8_t, 3>> in{{0, 128, 255}};
    std::vector<generic_color<uint16_t, 3>> out(in.size());
    pxl::convert(in.begin(), in.end(), out.begin(), 1.0f);
    REQUIRE(out[0][0] == 0);
    REQUIRE(out[0][1] == 32896);
    REQUIRE(out[0][2] == 65535);
}
TEST_CASE
(   "convert works on pxl::image objects with matching channel counts"
,   "[algorithm][convert]"
)
{   pxl::image<uint8_t, 3> in(2, 1);
    in[0][0] = generic_color<uint8_t, 3>{0, 128, 255};
    in[0][1] = generic_color<uint8_t, 3>{255, 0, 128};
    pxl::image<float, 3> out(2, 1);
    pxl::convert(in, out, 1.0f);
    REQUIRE_THAT(out[0][0][0], WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(out[0][0][1], WithinAbs(128.0f / 255.0f, 1e-6f));
    REQUIRE_THAT(out[0][0][2], WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(out[0][1][0], WithinAbs(1.0f, 1e-6f));
}
