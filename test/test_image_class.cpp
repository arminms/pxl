// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#include <pxl/image.hpp>
#include <pxl/image_view.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <numeric>

using color_type = pxl::color<uint8_t, 3>;
using image_type = pxl::image<uint8_t, 3>;
using view_type = pxl::image_view<uint8_t, 3>;

// -- ctors --------------------------------------------------------------------

TEST_CASE
(   "image constructor allocates width * height pixels"
,   "[image][ctors]"
)
{   image_type img(4, 3);
    REQUIRE(img.width() == 4);
    REQUIRE(img.height() == 3);
    REQUIRE(img.size() == 12);
}
TEST_CASE
(   "image fill constructor initializes every pixel to the given color"
,   "[image][ctors]"
)
{   color_type fill_color{10, 20, 30};
    image_type img(3, 2, fill_color);

    REQUIRE(img.width() == 3);
    REQUIRE(img.height() == 2);
    for (auto& p : img)
        REQUIRE(p == fill_color);
}
TEST_CASE
(   "image view constructor copies the pixels covered by a full-image view"
,   "[image][ctors]"
)
{   image_type src(3, 2, color_type{1, 2, 3});
    view_type view(src.data(), 0, 0, src.width(), src.height(), src.width());

    image_type img(view);
    REQUIRE(img.width() == 3);
    REQUIRE(img.height() == 2);
    for (auto& p : img)
        REQUIRE(p == color_type({1, 2, 3}));

    // the new image is independent from the source
    src.at(0) = color_type{9, 9, 9};
    REQUIRE(img.at(0) == color_type({1, 2, 3}));
}
TEST_CASE
(   "image view constructor copies only the pixels covered by a sub-region view"
,   "[image][ctors]"
)
{   image_type src(4, 3);
    for (image_type::size_type y = 0; y < src.height(); ++y)
        for (image_type::size_type x = 0; x < src.width(); ++x)
            src[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    // 2x2 sub-view starting at (1, 1)
    view_type view(src.data(), 1, 1, 2, 2, src.width());
    image_type img(view);

    REQUIRE(img.width() == 2);
    REQUIRE(img.height() == 2);
    REQUIRE(img[0][0] == color_type({1, 1, 0}));
    REQUIRE(img[0][1] == color_type({2, 1, 0}));
    REQUIRE(img[1][0] == color_type({1, 2, 0}));
    REQUIRE(img[1][1] == color_type({2, 2, 0}));
}
TEST_CASE
(   "image copy constructor duplicates the pixels"
,   "[image][ctors]"
)
{   image_type a(2, 2, color_type{1, 2, 3});
    image_type b(a);

    REQUIRE(b.width() == a.width());
    REQUIRE(b.height() == a.height());
    for (image_type::size_type i = 0; i < b.size(); ++i)
        REQUIRE(b.at(i) == a.at(i));

    // the copy is independent from the original
    a.at(0) = color_type{9, 9, 9};
    REQUIRE(b.at(0) == color_type({1, 2, 3}));
}
TEST_CASE
(   "image move constructor transfers the pixels"
,   "[image][ctors]"
)
{   image_type a(2, 2, color_type{4, 5, 6});
    image_type b(std::move(a));

    REQUIRE(b.width() == 2);
    REQUIRE(b.height() == 2);
    for (auto& p : b)
        REQUIRE(p == color_type({4, 5, 6}));
}

// -- assignment operators -----------------------------------------------------

TEST_CASE
(   "image copy assignment operator duplicates the pixels"
,   "[image][assignment]"
)
{   image_type a(2, 2, color_type{1, 2, 3});
    image_type b(1, 1, color_type{9, 9, 9});

    b = a;
    REQUIRE(b.width() == 2);
    REQUIRE(b.height() == 2);
    for (auto& p : b)
        REQUIRE(p == color_type({1, 2, 3}));

    // the copy is independent from the original
    a.at(0) = color_type{7, 7, 7};
    REQUIRE(b.at(0) == color_type({1, 2, 3}));
}
TEST_CASE
(   "image move assignment operator transfers the pixels"
,   "[image][assignment]"
)
{   image_type a(2, 2, color_type{4, 5, 6});
    image_type b(1, 1, color_type{9, 9, 9});

    b = std::move(a);
    REQUIRE(b.width() == 2);
    REQUIRE(b.height() == 2);
    for (auto& p : b)
        REQUIRE(p == color_type({4, 5, 6}));
}

// -- element access -------------------------------------------------------------

TEST_CASE
(   "image::operator[] returns a view of the specified row"
,   "[image][element_access]"
)
{   image_type img(3, 2);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    REQUIRE(img[0].width() == 3);
    REQUIRE(img[0].height() == 1);
    REQUIRE(img[1][2] == color_type({2, 1, 0}));

    const image_type cimg = img;
    REQUIRE(cimg[1][2] == color_type({2, 1, 0}));
}
TEST_CASE
(   "image::operator[][] allows writing through the row view"
,   "[image][element_access]"
)
{   image_type img(2, 2, color_type{1, 2, 3});

    img[0][0] = color_type{9, 9, 9};
    REQUIRE(img.at(0) == color_type({9, 9, 9}));
    REQUIRE(img[0][1] == color_type({1, 2, 3}));
}
TEST_CASE
(   "image::operator() returns a view of the specified sub-region"
,   "[image][element_access]"
)
{   image_type img(4, 3);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    auto sub = img(1, 1, 2, 2);
    REQUIRE(sub.width() == 2);
    REQUIRE(sub.height() == 2);
    REQUIRE(sub(0, 0) == color_type({1, 1, 0}));
    REQUIRE(sub(1, 0) == color_type({2, 1, 0}));
    REQUIRE(sub(0, 1) == color_type({1, 2, 0}));
    REQUIRE(sub(1, 1) == color_type({2, 2, 0}));

    // the view is a zero-copy window into the original image
    sub(0, 0) = color_type{9, 9, 9};
    REQUIRE(img[1][1] == color_type({9, 9, 9}));

    const image_type& cimg = img;
    auto csub = cimg(1, 1, 2, 2);
    REQUIRE(csub(0, 0) == color_type({9, 9, 9}));
}
TEST_CASE
(   "image::operator() defaults width/height to the rest of the image"
,   "[image][element_access]"
)
{   image_type img(4, 3);
    for (image_type::size_type y = 0; y < img.height(); ++y)
        for (image_type::size_type x = 0; x < img.width(); ++x)
            img[y][x] = color_type
            {   static_cast<uint8_t>(x)
            ,   static_cast<uint8_t>(y)
            ,   0
            };

    auto sub = img(1, 1);
    REQUIRE(sub.width() == 3);  // 4 - 1
    REQUIRE(sub.height() == 2); // 3 - 1
    REQUIRE(sub(0, 0) == color_type({1, 1, 0}));
    REQUIRE(sub(2, 1) == color_type({3, 2, 0}));

    const image_type& cimg = img;
    auto csub = cimg(2, 0);
    REQUIRE(csub.width() == 2);
    REQUIRE(csub.height() == 3);
    REQUIRE(csub(0, 0) == color_type({2, 0, 0}));
}
TEST_CASE
(   "image::at provides bounds-checked linear access"
,   "[image][element_access]"
)
{   image_type img(2, 2, color_type{1, 2, 3});

    REQUIRE(img.at(3) == color_type({1, 2, 3}));
    img.at(0) = color_type{9, 9, 9};
    REQUIRE(img.at(0) == color_type({9, 9, 9}));

    REQUIRE_THROWS_AS(img.at(4), std::out_of_range);

    const image_type cimg(2, 2, color_type{1, 2, 3});
    REQUIRE(cimg.at(3) == color_type({1, 2, 3}));
    REQUIRE_THROWS_AS(cimg.at(4), std::out_of_range);
}
TEST_CASE
(   "image::data provides direct access to the underlying storage"
,   "[image][element_access]"
)
{   image_type img(2, 2, color_type{1, 2, 3});
    REQUIRE(img.data() == &img.at(0));

    img.data()[1] = color_type{9, 9, 9};
    REQUIRE(img.at(1) == color_type({9, 9, 9}));

    const image_type cimg(2, 2, color_type{1, 2, 3});
    REQUIRE(cimg.data() == &cimg.at(0));
}

// -- iterators --------------------------------------------------------------

TEST_CASE
(   "image::begin/end iterate over the pixels in order"
,   "[image][iterators]"
)
{   image_type img(2, 2);
    int value = 0;
    for (auto it = img.begin(); it != img.end(); ++it, ++value)
        *it = color_type
        {   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        ,   static_cast<uint8_t>(value)
        };

    REQUIRE(std::distance(img.begin(), img.end()) == 4);
    REQUIRE(img.at(0) == color_type({0, 0, 0}));
    REQUIRE(img.at(3) == color_type({3, 3, 3}));
}
TEST_CASE
(   "image::cbegin/cend provide const iteration"
,   "[image][iterators]"
)
{   image_type img(2, 2, color_type{1, 2, 3});
    REQUIRE(std::all_of
    (   img.cbegin(), img.cend()
    ,   [](const color_type& p) { return p == color_type({1, 2, 3}); }
    ));
}
TEST_CASE
(   "image range-based for loop works with begin/end"
,   "[image][iterators]"
)
{   image_type img(2, 2, color_type{1, 1, 1});
    int count = 0;
    for (auto& p : img)
    {   REQUIRE(p == color_type({1, 1, 1}));
        ++count;
    }
    REQUIRE(count == 4);
}
TEST_CASE
(   "image::rbegin/rend iterate in reverse order"
,   "[image][iterators]"
)
{   image_type img(2, 2);
    int value = 0;
    for (auto& p : img)
        p = color_type
        {   static_cast<uint8_t>(value++)
        ,   0, 0
        };

    REQUIRE(std::equal
    (   img.rbegin(), img.rend()
    ,   std::vector<color_type>{
            color_type{3, 0, 0}
        ,   color_type{2, 0, 0}
        ,   color_type{1, 0, 0}
        ,   color_type{0, 0, 0}
        }.begin()
    ));

    *img.rbegin() = color_type{9, 9, 9};
    REQUIRE(img.at(3) == color_type({9, 9, 9}));
}
TEST_CASE
(   "image::crbegin/crend provide const reverse iteration"
,   "[image][iterators]"
)
{   image_type img(2, 1, color_type{5, 5, 5});
    const image_type& cimg = img;
    REQUIRE(std::all_of
    (   cimg.crbegin(), cimg.crend()
    ,   [](const color_type& p) { return p == color_type({5, 5, 5}); }
    ));
}

// -- capacity -----------------------------------------------------------------

TEST_CASE
(   "image::empty reports whether the image has any pixels"
,   "[image][capacity]"
)
{   image_type img(0, 0);
    REQUIRE(img.empty());

    image_type non_empty(1, 1);
    REQUIRE_FALSE(non_empty.empty());
}
TEST_CASE
(   "image::size, width and height report the image dimensions"
,   "[image][capacity]"
)
{   image_type img(5, 4);
    REQUIRE(img.width() == 5);
    REQUIRE(img.height() == 4);
    REQUIRE(img.size() == 20);
}
TEST_CASE
(   "image::channels reports the number of channels per pixel"
,   "[image][capacity]"
)
{   REQUIRE(image_type::channels() == 3);

    using rgba_pixel_type = pxl::color<uint8_t, 4>;
    using rgba_image_type = pxl::image<uint8_t, 4>;
    REQUIRE(rgba_image_type::channels() == 4);
}
