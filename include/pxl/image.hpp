// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <algorithm>

#include <pxl/concepts.hpp>
#include <pxl/color.hpp>
#include <pxl/image_view.hpp>
#include <pxl/memory.hpp>

namespace pxl {

template <Pixel PixelType, typename Container>
requires std::contiguous_iterator<typename Container::iterator>
struct generic_image
{   using value_type = typename Container::value_type;
    using pixel_type = PixelType;
    using channel_type = typename PixelType::channel_type;
    using size_type = typename Container::size_type;
    using difference_type = typename Container::difference_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using pointer = typename Container::pointer;
    using const_pointer = typename Container::const_pointer;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using reverse_iterator = typename Container::reverse_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

    /// @brief Sentinel used by `operator()` to mean "the rest of the image"
    /// for a `width`/`height` argument that was not provided.
    static constexpr size_type npos = static_cast<size_type>(-1);

// -- ctors --------------------------------------------------------------------
    ///
    /// @brief constructor
    /// @param width image width in pixel
    /// @param height image height in pixel
    generic_image
    (   size_type width
    ,   size_type height
    )   noexcept
    :   b_(width * height)
    ,   w_(width)
    ,   h_(height)
    {}
    /// @brief fill constructor
    /// @param width generic_image width in pixel
    /// @param height image height in pixel
    /// @param color fill color
    generic_image
    (   size_type width
    ,   size_type height
    ,   const generic_color<channel_type, pixel_type::max_size()>& color
    )
    :   b_(width * height)
    ,   w_(width)
    ,   h_(height)
    {   std::fill(begin(), end(), color);
    }
    /// @brief view constructor
    /// @param view image view to copy the pixels from
    generic_image(const generic_image_view<PixelType>& view)
    :   b_(std::begin(view), std::end(view))
    ,   w_(view.width())
    ,   h_(view.height())
    {}
    /// @brief copy constructor
    /// @param other image to copy from
    generic_image(const generic_image& other)
    :   b_(other.b_)
    ,   w_(other.w_)
    ,   h_(other.h_)
    {}
    /// @brief move constructor
    /// @param other image to move from
    generic_image(generic_image&& other)
    :   b_(std::move(other.b_))
    ,   w_(other.w_)
    ,   h_(other.h_)
    {}

// -- copy assignment operators ------------------------------------------------
    ///
    /// @brief copy assignment operator
    /// @param other The image to copy.
    generic_image& operator= (const generic_image& other)
    {   b_ = other.b_;
        w_ = other.w_;
        h_ = other.h_;
        return *this;
    }
    /// @brief move assignment operator
    /// @param other The image to move.
    generic_image& operator= (generic_image&& other)
    {   b_ = std::move(other.b_);
        w_ = other.w_;
        h_ = other.h_;
        return *this;
    }

// -- element access -----------------------------------------------------------
    ///
    /// @brief Get a view of the pixel row at the specified index, allowing
    /// `image[row][col]` access to a single pixel.
    /// @param row The index of the row to return.
    /// @return A view of the pixel row at the specified index.
    generic_image_view<PixelType> operator[] (size_type row) noexcept
    {   return generic_image_view<PixelType>(b_.data(), 0, row, w_, 1, w_);
    }
    /// @brief Get a view of the pixel row at the specified index, allowing
    /// `image[row][col]` access to a single pixel.
    /// @param row The index of the row to return.
    /// @return A const view of the pixel row at the specified index.
    generic_image_view<const PixelType> operator[] (size_type row) const noexcept
    {   return generic_image_view<const PixelType>(b_.data(), 0, row, w_, 1, w_);
    }
    /// @brief Returns a view of the specified sub-region ("sub-image") of
    /// the image.
    /// @param x Column offset of the sub-region's top-left corner.
    /// @param y Row offset of the sub-region's top-left corner.
    /// @param width Width of the sub-region in pixels; defaults to the
    ///     remaining width from `x` to the right edge of the image.
    /// @param height Height of the sub-region in pixels; defaults to the
    ///     remaining height from `y` to the bottom edge of the image.
    /// @return A view of the specified sub-region of the image.
    generic_image_view<PixelType> operator()
    (   size_type x
    ,   size_type y
    ,   size_type width = npos
    ,   size_type height = npos
    )   noexcept
    {   return generic_image_view<PixelType>
        (   b_.data()
        ,   x
        ,   y
        ,   width == npos ? w_ - x : width
        ,   height == npos ? h_ - y : height
        ,   w_
        );
    }
    /// @brief Returns a view of the specified sub-region ("sub-image") of
    /// the image.
    /// @param x Column offset of the sub-region's top-left corner.
    /// @param y Row offset of the sub-region's top-left corner.
    /// @param width Width of the sub-region in pixels; defaults to the
    ///     remaining width from `x` to the right edge of the image.
    /// @param height Height of the sub-region in pixels; defaults to the
    ///     remaining height from `y` to the bottom edge of the image.
    /// @return A const view of the specified sub-region of the image.
    generic_image_view<const PixelType> operator()
    (   size_type x
    ,   size_type y
    ,   size_type width = npos
    ,   size_type height = npos
    )   const noexcept
    {   return generic_image_view<const PixelType>
        (   b_.data()
        ,   x
        ,   y
        ,   width == npos ? w_ - x : width
        ,   height == npos ? h_ - y : height
        ,   w_
        );
    }
    /// @brief Get the pixel at the specified index with bounds checking.
    /// @param idx The index of the pixel to return.
    /// @return Reference to the pixel at the specified index.
    /// @throws std::out_of_range if `idx >= size()`.
    reference at(size_type idx)
    {   if (idx >= b_.size())
            throw std::out_of_range("generic_image::at: index out of range");
        return b_[idx];
    }
    /// @brief Get the pixel at the specified index with bounds checking.
    /// @param idx The index of the pixel to return.
    /// @return Const reference to the pixel at the specified index.
    /// @throws std::out_of_range if `idx >= size()`.
    const_reference at(size_type idx) const
    {   if (idx >= b_.size())
            throw std::out_of_range("generic_image::at: index out of range");
        return b_[idx];
    }
    /// @brief Get a pointer to the underlying data.
    /// @return A pointer to the underlying data.
    pointer data() noexcept
    {   return b_.data();
    }
    /// @brief Get a const pointer to the underlying data.
    /// @return A const pointer to the underlying data.
    const_pointer data() const noexcept
    {   return b_.data();
    }

// -- iterators --------------------------------------------------------------
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Iterator to the first channel.
    constexpr iterator begin() noexcept
    {   return b_.begin();
    }
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Const iterator to the first channel.
    constexpr const_iterator begin() const noexcept
    {   return b_.begin();
    }
    ///
    /// @brief Returns a const iterator to the beginning
    /// @return Const iterator to the first channel.
    constexpr const_iterator cbegin() const noexcept
    {   return b_.cbegin();
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Iterator to the channel following the last channel.
    constexpr iterator end() noexcept
    {   return b_.end();
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Const iterator to the channel following the last channel.
    constexpr const_iterator end() const noexcept
    {   return b_.end();
    }
    ///
    /// @brief Returns a const iterator to the end
    /// @return Const iterator to the channel following the last channel.
    constexpr const_iterator cend() const noexcept
    {   return b_.cend();
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr reverse_iterator rbegin() noexcept
    {   return b_.rbegin();
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Const reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr const_reverse_iterator rbegin() const noexcept
    {   return b_.rbegin();
    }
    ///
    /// @brief Returns a const reverse iterator to the beginning
    /// @return Const reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr const_reverse_iterator crbegin() const noexcept
    {   return b_.crbegin();
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Reverse iterator to the channel following the last channel
    ///     of the reversed color (i.e. preceding the first channel).
    constexpr reverse_iterator rend() noexcept
    {   return b_.rend();
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Const reverse iterator to the channel following the last
    ///     channel of the reversed color (i.e. preceding the first channel).
    constexpr const_reverse_iterator rend() const noexcept
    {   return b_.rend();
    }
    ///
    /// @brief Returns a const reverse iterator to the end
    /// @return Const reverse iterator to the channel following the last
    ///     channel of the reversed color (i.e. preceding the first channel).
    constexpr const_reverse_iterator crend() const noexcept
    {   return b_.crend();
    }

// -- capacity -----------------------------------------------------------------
    ///
    /// @brief Checks whether the image is empty (i.e. has no pixels).
    /// @return `true` if the image has no pixels, `false` otherwise.
    [[nodiscard]] bool empty() const noexcept
    {   return b_.empty();
    }
    ///
    /// @brief Returns the number of pixels in the image.
    /// @return The number of pixels in the image.
    [[nodiscard]] constexpr size_type size() const noexcept
    {   return b_.size();
    }
    /// @brief Get the width of the image.
    /// @return The width of the image.
    size_type width() const noexcept
    {   return w_;
    }
    /// @brief Get the height of the image.
    /// @return The height of the image.
    size_type height() const noexcept
    {   return h_;
    }
    /// @brief Get the number of channels in the image.
    /// @return The number of channels in the image.
    constexpr static size_type channels() noexcept
    {   return pixel_type::max_size();
    }

private:
    Container  b_; // buffer
    size_type  w_; // width
    size_type  h_; // height
};

/// @brief A convenience alias for an image with a default type and number of
/// channels.
/// @tparam T The type of each channel.
/// @tparam C The number of channels.
template <typename T, std::size_t C = 4>
using image
=   generic_image<generic_color<T, C>
,   std::vector<generic_color<T, C>>>; 
/// @brief A convenience alias for an image with a default type and number of
/// channels, without initialization.
/// @tparam T The type of each channel.
/// @tparam C The number of channels.
template <typename T = uint8_t, std::size_t C = 4>
using image_no_init
=   generic_image<generic_color<T, C>
,   std::vector<generic_color<T, C>
,   no_init_allocator<generic_color<T, C>>>>; 

} // end pxl namespace