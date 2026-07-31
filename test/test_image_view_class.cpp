// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#include <pxl/image_view.hpp>
#include <pxl/image.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <type_traits>

using color_type = pxl::color<uint8_t, 3>;
using image_type = pxl::image<uint8_t, 3>;
using view_type = pxl::image_view<uint8_t, 3>;

// -- concepts -----------------------------------------------------------------

TEST_CASE
(   "generic_image_view satisfies the std::ranges::view concept"
,   "[image_view][concepts]"
)
{   STATIC_REQUIRE(std::ranges::view<view_type>);
    STATIC_REQUIRE(std::ranges::random_access_range<view_type>);
    STATIC_REQUIRE(std::ranges::sized_range<view_type>);
    STATIC_REQUIRE(std::ranges::common_range<view_type>);
    STATIC_REQUIRE
    (   std::derived_from
        <   view_type
        ,   std::ranges::view_interface<view_type>
        >
    );
    STATIC_REQUIRE(std::random_access_iterator<view_type::iterator>);
    STATIC_REQUIRE(std::random_access_iterator<view_type::const_iterator>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<view_type>);
}

// -- ctors --------------------------------------------------------------------

TEST_CASE
(   "image_view default constructor produces an empty view"
,   "[image_view][ctors]"
)
{   view_type v;
    REQUIRE(v.empty());
    REQUIRE(v.size() == 0);
    REQUIRE(v.width() == 0);
    REQUIRE(v.height() == 0);
    REQUIRE(v.begin() == v.end());
}
TEST_CASE
(   "image_view constructor covers the whole image when stride equals width"
,   "[image_view][ctors]"
)
{   image_type img(4, 3, color_type{1, 2, 3});
    view_type v(img.data(), 0, 0, img.width(), img.height(), img.width());

    REQUIRE(v.width() == 4);
    REQUIRE(v.height() == 3);
    REQUIRE(v.stride() == 4);
    REQUIRE(v.size() == 12);
    REQUIRE(v.data() == img.data());
    for (auto& p : v)
        REQUIRE(p == color_type({1, 2, 3}));
}
TEST_CASE
(   "image_view constructor with x, y offsets a sub-region of a larger image"
,   "[image_view][ctors]"
)
{   image_type img(4, 3);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    // 2x2 sub-view starting at (1, 1)
    view_type v(img.data(), 1, 1, 2, 2, img.width());

    REQUIRE(v.width() == 2);
    REQUIRE(v.height() == 2);
    REQUIRE(v.stride() == 4);
    REQUIRE(v(0, 0) == color_type({1, 1, 0}));
    REQUIRE(v(1, 0) == color_type({2, 1, 0}));
    REQUIRE(v(0, 1) == color_type({1, 2, 0}));
    REQUIRE(v(1, 1) == color_type({2, 2, 0}));
}

// -- zero-copy semantics --------------------------------------------------------

TEST_CASE
(   "image_view writes through to the underlying image storage"
,   "[image_view][semantics]"
)
{   image_type img(3, 3, color_type{0, 0, 0});
    view_type v(img.data(), 1, 1, 2, 2, img.width());

    v(0, 0) = color_type{9, 9, 9};
    REQUIRE(img[1][1] == color_type({9, 9, 9}));

    for (auto& p : v)
        p = color_type{5, 5, 5};
    REQUIRE(img[1][1] == color_type({5, 5, 5}));
    REQUIRE(img[1][2] == color_type({5, 5, 5}));
    REQUIRE(img[2][1] == color_type({5, 5, 5}));
    REQUIRE(img[2][2] == color_type({5, 5, 5}));
    // pixels outside the view are untouched
    REQUIRE(img[0][0] == color_type({0, 0, 0}));
    REQUIRE(img[1][0] == color_type({0, 0, 0}));
}

// -- element access -------------------------------------------------------------

TEST_CASE
(   "image_view::operator[] provides unchecked linear row-major access"
,   "[image_view][element_access]"
)
{   image_type img(4, 3);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    view_type v(img.data(), 1, 0, 2, 2, img.width());
    REQUIRE(v[0] == color_type({1, 0, 0}));
    REQUIRE(v[1] == color_type({2, 0, 0}));
    REQUIRE(v[2] == color_type({1, 1, 0}));
    REQUIRE(v[3] == color_type({2, 1, 0}));

    const view_type cv = v;
    REQUIRE(cv[3] == color_type({2, 1, 0}));
}
TEST_CASE
(   "image_view::at provides bounds-checked (x, y) access"
,   "[image_view][element_access]"
)
{   image_type img(4, 4, color_type{1, 2, 3});
    view_type v(img.data(), 1, 1, 2, 2, img.width());

    REQUIRE(v.at(1, 1) == color_type({1, 2, 3}));
    v.at(0, 0) = color_type{9, 9, 9};
    REQUIRE(img[1][1] == color_type({9, 9, 9}));

    REQUIRE_THROWS_AS(v.at(2, 0), std::out_of_range);
    REQUIRE_THROWS_AS(v.at(0, 2), std::out_of_range);

    const view_type cv = v;
    REQUIRE(cv.at(1, 1) == color_type({1, 2, 3}));
    REQUIRE_THROWS_AS(cv.at(2, 0), std::out_of_range);
}
TEST_CASE
(   "image_view::data provides direct access to the underlying storage"
,   "[image_view][element_access]"
)
{   image_type img(4, 4, color_type{1, 2, 3});
    view_type v(img.data(), 1, 1, 2, 2, img.width());
    REQUIRE(v.data() == &img[1][1]);

    const view_type cv = v;
    REQUIRE(cv.data() == &img[1][1]);
}

// -- iterators --------------------------------------------------------------

TEST_CASE
(   "image_view::begin/end iterate over the pixels in row-major order"
,   "[image_view][iterators]"
)
{   image_type img(4, 3);
    view_type v(img.data(), 1, 0, 2, 2, img.width());

    REQUIRE(std::distance(v.begin(), v.end()) == 4);

    int value = 0;
    for (auto it = v.begin(); it != v.end(); ++it, ++value)
        *it = color_type
        {   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        };

    REQUIRE(img[0][1] == color_type({0, 0, 0}));
    REQUIRE(img[0][2] == color_type({1, 1, 1}));
    REQUIRE(img[1][1] == color_type({2, 2, 2}));
    REQUIRE(img[1][2] == color_type({3, 3, 3}));
}
TEST_CASE
(   "image_view iterator supports random access arithmetic"
,   "[image_view][iterators]"
)
{   image_type img(4, 4);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    view_type v(img.data(), 0, 0, 3, 3, img.width());
    auto it = v.begin();

    REQUIRE(*(it + 4) == color_type({1, 1, 0})); // row-major index 4 => (1, 1)
    it += 5;
    REQUIRE(*it == color_type({2, 1, 0}));
    --it;
    REQUIRE(*it == color_type({1, 1, 0}));
    REQUIRE((v.end() - v.begin()) == 9);
    REQUIRE(v.begin() < v.end());
}
TEST_CASE
(   "image_view::cbegin/cend provide const iteration"
,   "[image_view][iterators]"
)
{   image_type img(3, 2, color_type{1, 2, 3});
    view_type v(img.data(), 0, 0, 3, 2, img.width());

    REQUIRE(std::all_of
    (   v.cbegin(), v.cend()
    ,   [](const color_type& p) { return p == color_type({1, 2, 3}); }
    ));
}
TEST_CASE
(   "image_view::rbegin/rend iterate over the pixels in reverse order"
,   "[image_view][iterators]"
)
{   image_type img(2, 2);
    view_type v(img.data(), 0, 0, 2, 2, img.width());
    int value = 0;
    for (auto it = v.begin(); it != v.end(); ++it, ++value)
        *it = color_type
        {   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        };

    auto rit = v.rbegin();
    REQUIRE(*rit == color_type({3, 3, 3}));
    ++rit;
    REQUIRE(*rit == color_type({2, 2, 2}));
    REQUIRE(std::distance(v.rbegin(), v.rend()) == 4);
}
TEST_CASE
(   "image_view range-based for loop works with begin/end"
,   "[image_view][iterators]"
)
{   image_type img(2, 2, color_type{1, 1, 1});
    view_type v(img.data(), 0, 0, 2, 2, img.width());

    for (auto& p : v)
        REQUIRE(p == color_type({1, 1, 1}));
}

// -- capacity -----------------------------------------------------------------

TEST_CASE
(   "image_view::empty reports whether the view has any pixels"
,   "[image_view][capacity]"
)
{   image_type img(2, 2);
    view_type empty_view;
    view_type non_empty_view(img.data(), 0, 0, 2, 2, img.width());

    REQUIRE(empty_view.empty());
    REQUIRE_FALSE(non_empty_view.empty());
}
TEST_CASE
(   "image_view::channels returns the pixel type's channel count"
,   "[image_view][capacity]"
)
{   REQUIRE(view_type::channels() == 3);
}
