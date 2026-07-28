// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#include <pxl/color.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <numeric>
#include <stdexcept>
#include <utility>

using pxl::generic_color;

// -- ctors ------------------------------------------------------------------

TEST_CASE("generic_color default constructor zero-initializes channels", "[color][ctors]")
{
    generic_color<float, 4> c;
    REQUIRE(c.size() == 4);
    for (auto v : c)
        REQUIRE(v == 0.0f);
}

TEST_CASE("generic_color array constructor copies from a std::array", "[color][ctors]")
{
    std::array<float, 4> a{1.f, 2.f, 3.f, 4.f};
    generic_color<float, 4> c(a);
    REQUIRE(c[0] == 1.f);
    REQUIRE(c[1] == 2.f);
    REQUIRE(c[2] == 3.f);
    REQUIRE(c[3] == 4.f);
}

TEST_CASE("generic_color iterator constructor copies the range", "[color][ctors]")
{
    std::array<int, 3> src{10, 20, 30};
    generic_color<int, 3> c(src.begin(), src.end());
    REQUIRE(c[0] == 10);
    REQUIRE(c[1] == 20);
    REQUIRE(c[2] == 30);
}

TEST_CASE("generic_color copy constructor duplicates the channels", "[color][ctors]")
{
    generic_color<int, 3> a{1, 2, 3};
    generic_color<int, 3> b(a);
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
    REQUIRE(b[2] == 3);

    // the copy is independent from the original
    a[0] = 99;
    REQUIRE(b[0] == 1);
}

TEST_CASE("generic_color move constructor transfers the channels", "[color][ctors]")
{
    generic_color<int, 3> a{1, 2, 3};
    generic_color<int, 3> b(std::move(a));
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
    REQUIRE(b[2] == 3);
}

TEST_CASE("generic_color initializer list constructor sets the channels", "[color][ctors]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    REQUIRE(c[0] == 1);
    REQUIRE(c[1] == 2);
    REQUIRE(c[2] == 3);
    REQUIRE(c[3] == 4);
}

// -- element access -----------------------------------------------------------

TEST_CASE("generic_color::at provides bounds-checked access", "[color][element_access]")
{
    generic_color<int, 4> c{1, 2, 3, 4};

    REQUIRE(c.at(0) == 1);
    REQUIRE(c.at(3) == 4);
    c.at(1) = 42;
    REQUIRE(c[1] == 42);

    REQUIRE_THROWS_AS(c.at(4), std::out_of_range);

    const generic_color<int, 4> cc{1, 2, 3, 4};
    REQUIRE(cc.at(2) == 3);
    REQUIRE_THROWS_AS(cc.at(10), std::out_of_range);
}

TEST_CASE("generic_color::operator[] provides unchecked access", "[color][element_access]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    REQUIRE(c[0] == 1);
    c[0] = 100;
    REQUIRE(c[0] == 100);

    const generic_color<int, 4> cc{1, 2, 3, 4};
    REQUIRE(cc[3] == 4);
}

TEST_CASE("generic_color::front and back access the first and last channels", "[color][element_access]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    REQUIRE(c.front() == 1);
    REQUIRE(c.back() == 4);

    c.front() = 10;
    c.back() = 40;
    REQUIRE(c[0] == 10);
    REQUIRE(c[3] == 40);

    const generic_color<int, 4> cc{1, 2, 3, 4};
    REQUIRE(cc.front() == 1);
    REQUIRE(cc.back() == 4);
}

TEST_CASE("generic_color::data provides direct access to the underlying storage", "[color][element_access]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    REQUIRE(c.data() == &c[0]);
    c.data()[1] = 99;
    REQUIRE(c[1] == 99);

    const generic_color<int, 4> cc{1, 2, 3, 4};
    REQUIRE(cc.data() == &cc[0]);
}

// -- iterators ----------------------------------------------------------------

TEST_CASE("generic_color::begin/end iterate over the channels in order", "[color][iterators]")
{
    generic_color<int, 4> c{1, 2, 3, 4};

    REQUIRE(std::distance(c.begin(), c.end()) == 4);
    int expected = 1;
    for (auto it = c.begin(); it != c.end(); ++it, ++expected)
        REQUIRE(*it == expected);

    *c.begin() = 100;
    REQUIRE(c[0] == 100);
}

TEST_CASE("generic_color::cbegin/cend provide const iteration", "[color][iterators]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    REQUIRE(std::equal(c.cbegin(), c.cend(), std::array<int,4>{1,2,3,4}.begin()));
}

TEST_CASE("generic_color range-based for loop works with begin/end", "[color][iterators]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    int sum = 0;
    for (auto v : c)
        sum += v;
    REQUIRE(sum == 10);
}

TEST_CASE("generic_color::rbegin/rend iterate in reverse order", "[color][iterators]")
{
    generic_color<int, 4> c{1, 2, 3, 4};

    std::array<int, 4> reversed{};
    std::copy(c.rbegin(), c.rend(), reversed.begin());
    REQUIRE(reversed == std::array<int, 4>{4, 3, 2, 1});

    *c.rbegin() = 400;
    REQUIRE(c[3] == 400);
}

TEST_CASE("generic_color::crbegin/crend provide const reverse iteration", "[color][iterators]")
{
    const generic_color<int, 4> c{1, 2, 3, 4};
    std::array<int, 4> reversed{};
    std::copy(c.crbegin(), c.crend(), reversed.begin());
    REQUIRE(reversed == std::array<int, 4>{4, 3, 2, 1});
}

// -- capacity -------------------------------------------------------------------

TEST_CASE("generic_color::empty reports whether there are any channels", "[color][capacity]")
{
    generic_color<int, 4> c;
    REQUIRE_FALSE(c.empty());

    generic_color<int, 0> empty_c;
    REQUIRE(empty_c.empty());
}

TEST_CASE("generic_color::size and max_size report the number of channels", "[color][capacity]")
{
    generic_color<int, 4> c;
    REQUIRE(c.size() == 4);
    REQUIRE(c.max_size() == 4);

    generic_color<double, 3> d;
    REQUIRE(d.size() == 3);
    REQUIRE(d.max_size() == 3);
}

// -- operations -----------------------------------------------------------------

TEST_CASE("generic_color::fill assigns the same value to every channel", "[color][operations]")
{
    generic_color<int, 4> c{1, 2, 3, 4};
    c.fill(7);
    for (auto v : c)
        REQUIRE(v == 7);
}

TEST_CASE("generic_color::swap exchanges the channels of two colors", "[color][operations]")
{
    generic_color<int, 4> a{1, 2, 3, 4};
    generic_color<int, 4> b{5, 6, 7, 8};

    a.swap(b);

    REQUIRE(a[0] == 5);
    REQUIRE(a[1] == 6);
    REQUIRE(a[2] == 7);
    REQUIRE(a[3] == 8);
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
    REQUIRE(b[2] == 3);
    REQUIRE(b[3] == 4);
}

// -- comparisons ------------------------------------------------------------------

TEST_CASE("generic_color equality and inequality compare all channels", "[color][comparisons]")
{
    generic_color<int, 4> a{1, 2, 3, 4};
    generic_color<int, 4> b{1, 2, 3, 4};
    generic_color<int, 4> c{1, 2, 3, 5};

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("generic_color relational operators use lexicographical order", "[color][comparisons]")
{
    generic_color<int, 4> a{1, 2, 3, 4};
    generic_color<int, 4> b{1, 2, 3, 4};
    generic_color<int, 4> c{1, 2, 3, 5};

    REQUIRE(a < c);
    REQUIRE(c > a);
    REQUIRE(a <= b);
    REQUIRE(a >= b);
    REQUIRE(a <= c);
    REQUIRE(c >= a);
    REQUIRE_FALSE(c < a);
}

TEST_CASE("uint32_t converting ctor 8-bits per channel", "[color][ctor][uint8_t]")
{   pxl::generic_color<uint8_t> c(0x1a2b3c4d);
    CHECK(0x1a == c[0]);
    CHECK(0x2b == c[1]);
    CHECK(0x3c == c[2]);
    CHECK(0x4d == c[3]);
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}

TEST_CASE("uint32_t converting ctor 16-bits per channel", "[color][ctor][uint16_t]")
{   pxl::generic_color<uint16_t> c(0x1a2b3c4d);
    CHECK(0x1a * 0xFF == c[0]);
    CHECK(0x2b * 0xFF == c[1]);
    CHECK(0x3c * 0xFF == c[2]);
    CHECK(0x4d * 0xFF == c[3]);
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}

TEST_CASE("uint32_t converting ctor 32-bits per channel", "[color][ctor][uint32_t]")
{   pxl::generic_color<uint32_t> c(0x1a2b3c4d);
    CHECK(0x1a * 0xFFFF == c[0]);
    CHECK(0x2b * 0xFFFF == c[1]);
    CHECK(0x3c * 0xFFFF == c[2]);
    CHECK(0x4d * 0xFFFF == c[3]);
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}

TEST_CASE("uint32_t converting ctor 64-bits per channel", "[color][ctor][uint64_t]")
{   pxl::generic_color<uint64_t> c(0x1a2b3c4d);
    CHECK(0x1a * 0xFFFFFF == c[0]);
    CHECK(0x2b * 0xFFFFFF == c[1]);
    CHECK(0x3c * 0xFFFFFF == c[2]);
    CHECK(0x4d * 0xFFFFFF == c[3]);
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}

TEST_CASE("uint32_t converting ctor float per channel", "[color][ctor][float]")
{   pxl::generic_color<float> c(0x1a2b3c4d);
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.101961, 0.0001));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(0.168627, 0.0001));
    REQUIRE_THAT(c[2], Catch::Matchers::WithinRel(0.235294, 0.0001));
    REQUIRE_THAT(c[3], Catch::Matchers::WithinRel(0.301961, 0.0001));
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}

TEST_CASE("uint32_t converting ctor double per channel", "[color][ctor][double]")
{   pxl::generic_color<double> c(0x1a2b3c4d);
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.101961, 0.0001));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(0.168627, 0.0001));
    REQUIRE_THAT(c[2], Catch::Matchers::WithinRel(0.235294, 0.0001));
    REQUIRE_THAT(c[3], Catch::Matchers::WithinRel(0.301961, 0.0001));
    CHECK(c == decltype(c)(0x1a2b3c4d_rgba));
}
