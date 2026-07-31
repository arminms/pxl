// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <compare>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>

#include <pxl/concepts.hpp>
#include <pxl/color.hpp>

namespace pxl {

/// @brief A non-owning, zero-copy view over a rectangular region of pixel
/// data stored elsewhere, modeled after `std::string_view`. It satisfies
/// `std::ranges::view` and can be iterated/indexed like a `generic_image`.
/// @tparam PixelType The pixel type of the viewed data.
template <Pixel PixelType>
class generic_image_view
:   public std::ranges::view_interface<generic_image_view<PixelType>>
{
public:
    using pixel_type = PixelType;
    using channel_type = typename PixelType::channel_type;
    using value_type = PixelType;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = PixelType&;
    using const_reference = const PixelType&;
    using pointer = PixelType*;
    using const_pointer = const PixelType*;

private:
    // -- iterator -------------------------------------------------------------
    // random access iterator over a (possibly strided) rectangular region;
    // the linear index is mapped to a row/col pair on every dereference so
    // that no per-row bookkeeping is needed when advancing/rewinding by n.
    template <bool Const>
    class basic_iterator
    {
    public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = PixelType;
        using difference_type   = std::ptrdiff_t;
        using pointer
        =   std::conditional_t<Const, const PixelType*, PixelType*>;
        using reference
        =   std::conditional_t<Const, const PixelType&, PixelType&>;

        basic_iterator() noexcept = default;
        /// @brief converting constructor from iterator to const_iterator
        template <bool OtherConst>
        requires (Const && !OtherConst)
        basic_iterator(const basic_iterator<OtherConst>& other) noexcept
        :   base_(other.base_)
        ,   width_(other.width_)
        ,   stride_(other.stride_)
        ,   idx_(other.idx_)
        {}

        reference operator* () const noexcept
        {   return base_[row() * stride_ + col()];
        }
        pointer operator-> () const noexcept
        {   return &base_[row() * stride_ + col()];
        }
        reference operator[] (difference_type n) const noexcept
        {   return *(*this + n);
        }
        basic_iterator& operator++ () noexcept
        {   ++idx_;
            return *this;
        }
        basic_iterator operator++ (int) noexcept
        {   auto tmp = *this;
            ++*this;
            return tmp;
        }
        basic_iterator& operator-- () noexcept
        {   --idx_;
            return *this;
        }
        basic_iterator operator-- (int) noexcept
        {   auto tmp = *this;
            --*this;
            return tmp;
        }
        basic_iterator& operator+= (difference_type n) noexcept
        {   idx_ += n;
            return *this;
        }
        basic_iterator& operator-= (difference_type n) noexcept
        {   idx_ -= n;
            return *this;
        }
        friend basic_iterator operator+
        (   basic_iterator it
        ,   difference_type n
        )   noexcept
        {   it += n;
            return it;
        }
        friend basic_iterator operator+
        (   difference_type n
        ,   basic_iterator it
        )   noexcept
        {   it += n;
            return it;
        }
        friend basic_iterator operator-
        (   basic_iterator it
        ,   difference_type n
        )   noexcept
        {   it -= n;
            return it;
        }
        friend difference_type operator-
        (   const basic_iterator& a
        ,   const basic_iterator& b
        )   noexcept
        {   return a.idx_ - b.idx_;
        }
        friend bool operator==
        (   const basic_iterator& a
        ,   const basic_iterator& b
        )   noexcept
        {   return a.idx_ == b.idx_;
        }
        friend auto operator<=>
        (   const basic_iterator& a
        ,   const basic_iterator& b
        )   noexcept
        {   return a.idx_ <=> b.idx_;
        }

    private:
        friend class generic_image_view;
        template <bool> friend class basic_iterator;

        basic_iterator
        (   pointer base
        ,   size_type width
        ,   size_type stride
        ,   difference_type index
        )   noexcept
        :   base_(base)
        ,   width_(width)
        ,   stride_(stride)
        ,   idx_(index)
        {}

        size_type row() const noexcept
        {   return static_cast<size_type>(idx_) / width_;
        }
        size_type col() const noexcept
        {   return static_cast<size_type>(idx_) % width_;
        }

        pointer         base_ = nullptr;
        size_type       width_ = 0;
        size_type       stride_ = 0;
        difference_type idx_ = 0;
    };

public:
    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

// -- ctors --------------------------------------------------------------------
    ///
    /// @brief Default constructor
    /// @details Constructs an empty view.
    generic_image_view() noexcept = default;
    /// @brief constructor
    /// @param data Pointer to the origin (0, 0) of the underlying pixel
    ///     buffer being viewed.
    /// @param x Column offset of the view's top-left corner in the buffer.
    /// @param y Row offset of the view's top-left corner in the buffer.
    /// @param width Width of the view in pixels.
    /// @param height Height of the view in pixels.
    /// @param stride Number of pixels between the start of consecutive rows
    ///     in the underlying buffer (may differ from `width` for a view
    ///     over a sub-region of a larger image).
    generic_image_view
    (   pointer data
    ,   size_type x
    ,   size_type y
    ,   size_type width
    ,   size_type height
    ,   size_type stride
    )   noexcept
    :   p_(data + y * stride + x)
    ,   w_(width)
    ,   h_(height)
    ,   stride_(stride)
    {}

// -- element access -----------------------------------------------------------
    ///
    /// @brief Get the pixel at the specified index (row-major order).
    /// @param idx The index of the pixel to return.
    /// @return Reference to the pixel at the specified index.
    reference operator[] (size_type idx) noexcept
    {   return p_[(idx / w_) * stride_ + (idx % w_)];
    }
    /// @brief Get the pixel at the specified index (row-major order).
    /// @param idx The index of the pixel to return.
    /// @return Const reference to the pixel at the specified index.
    const_reference operator[] (size_type idx) const noexcept
    {   return p_[(idx / w_) * stride_ + (idx % w_)];
    }
    /// @brief Returns the pixel at the specified position from the top left
    /// corner of the view.
    /// @param x Column index from the top left corner of the view.
    /// @param y row index from the top left corner of the view.
    /// @return The pixel at the specified position in the view.
    reference operator() (size_type x, size_type y) noexcept
    {   return p_[y * stride_ + x];
    }
    /// @brief Returns the pixel at the specified position from the top left
    /// corner of the view.
    /// @param x Column index from the top left corner of the view.
    /// @param y row index from the top left corner of the view.
    /// @return Const reference to the pixel at the specified position.
    const_reference operator() (size_type x, size_type y) const noexcept
    {   return p_[y * stride_ + x];
    }
    /// @brief Get the pixel at the specified position with bounds checking.
    /// @param x The x-coordinate of the pixel to return.
    /// @param y The y-coordinate of the pixel to return.
    /// @return Reference to the pixel at the specified position.
    /// @throws std::out_of_range if `x >= width() || y >= height()`.
    reference at(size_type x, size_type y)
    {   if (x >= w_ || y >= h_)
            throw std::out_of_range("generic_image_view::at: index out of range");
        return p_[y * stride_ + x];
    }
    /// @brief Get the pixel at the specified position with bounds checking.
    /// @param x The x-coordinate of the pixel to return.
    /// @param y The y-coordinate of the pixel to return.
    /// @return Const reference to the pixel at the specified position.
    /// @throws std::out_of_range if `x >= width() || y >= height()`.
    const_reference at(size_type x, size_type y) const
    {   if (x >= w_ || y >= h_)
            throw std::out_of_range("generic_image_view::at: index out of range");
        return p_[y * stride_ + x];
    }
    /// @brief Get a pointer to the first pixel of the view.
    /// @return A pointer to the first pixel of the view.
    /// @note The underlying memory is contiguous only when `stride() ==
    ///     width()`; otherwise consecutive rows are separated by padding.
    pointer data() noexcept
    {   return p_;
    }
    /// @brief Get a const pointer to the first pixel of the view.
    /// @return A const pointer to the first pixel of the view.
    /// @note The underlying memory is contiguous only when `stride() ==
    ///     width()`; otherwise consecutive rows are separated by padding.
    const_pointer data() const noexcept
    {   return p_;
    }

// -- iterators --------------------------------------------------------------
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Iterator to the first pixel.
    iterator begin() noexcept
    {   return iterator(p_, w_, stride_, 0);
    }
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Const iterator to the first pixel.
    const_iterator begin() const noexcept
    {   return const_iterator(p_, w_, stride_, 0);
    }
    ///
    /// @brief Returns a const iterator to the beginning
    /// @return Const iterator to the first pixel.
    const_iterator cbegin() const noexcept
    {   return begin();
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Iterator to the pixel following the last pixel.
    iterator end() noexcept
    {   return iterator(p_, w_, stride_, static_cast<difference_type>(w_ * h_));
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Const iterator to the pixel following the last pixel.
    const_iterator end() const noexcept
    {   return const_iterator(p_, w_, stride_, static_cast<difference_type>(w_ * h_));
    }
    ///
    /// @brief Returns a const iterator to the end
    /// @return Const iterator to the pixel following the last pixel.
    const_iterator cend() const noexcept
    {   return end();
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Reverse iterator to the first pixel of the reversed view
    ///     (i.e. the last pixel).
    reverse_iterator rbegin() noexcept
    {   return reverse_iterator(end());
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Const reverse iterator to the first pixel of the reversed
    ///     view (i.e. the last pixel).
    const_reverse_iterator rbegin() const noexcept
    {   return const_reverse_iterator(end());
    }
    ///
    /// @brief Returns a const reverse iterator to the beginning
    /// @return Const reverse iterator to the first pixel of the reversed
    ///     view (i.e. the last pixel).
    const_reverse_iterator crbegin() const noexcept
    {   return rbegin();
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Reverse iterator to the pixel preceding the first pixel.
    reverse_iterator rend() noexcept
    {   return reverse_iterator(begin());
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Const reverse iterator to the pixel preceding the first
    ///     pixel.
    const_reverse_iterator rend() const noexcept
    {   return const_reverse_iterator(begin());
    }
    ///
    /// @brief Returns a const reverse iterator to the end
    /// @return Const reverse iterator to the pixel preceding the first
    ///     pixel.
    const_reverse_iterator crend() const noexcept
    {   return rend();
    }

// -- capacity -----------------------------------------------------------------
    ///
    /// @brief Checks whether the view is empty (i.e. has no pixels).
    /// @return `true` if the view has no pixels, `false` otherwise.
    [[nodiscard]] bool empty() const noexcept
    {   return w_ == 0 || h_ == 0;
    }
    ///
    /// @brief Returns the number of pixels in the view.
    /// @return The number of pixels in the view.
    [[nodiscard]] constexpr size_type size() const noexcept
    {   return w_ * h_;
    }
    /// @brief Get the width of the view.
    /// @return The width of the view.
    size_type width() const noexcept
    {   return w_;
    }
    /// @brief Get the height of the view.
    /// @return The height of the view.
    size_type height() const noexcept
    {   return h_;
    }
    /// @brief Get the stride of the underlying buffer.
    /// @return The number of pixels between the start of consecutive rows
    ///     in the underlying buffer.
    size_type stride() const noexcept
    {   return stride_;
    }
    /// @brief Get the number of channels in the pixel type.
    /// @return The number of channels in the pixel type.
    constexpr static size_type channels() noexcept
    {   return pixel_type::max_size();
    }

private:
    pointer   p_ = nullptr; // pointer to the view's (0, 0) pixel
    size_type w_ = 0;       // width
    size_type h_ = 0;       // height
    size_type stride_ = 0;  // pixels per row in the underlying buffer
};

/// @brief A convenience alias for a view over an image with a default type
/// and number of channels.
/// @tparam T The type of each channel.
/// @tparam C The number of channels.
template <typename T, std::size_t C = 4>
using image_view = generic_image_view<generic_color<T, C>>;

} // end pxl namespace
