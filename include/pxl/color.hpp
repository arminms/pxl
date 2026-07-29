// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Armin Sobhani
//
#pragma once

#include <array>
#include <type_traits>

#include <pxl/detail/color.hpp>

namespace pxl {

template <typename T, std::size_t C = 4>
struct generic_color
{
public:
    using value_type = T;
    using size_type = typename std::array<T,C>::size_type;
    using difference_type = typename std::array<T,C>::difference_type;
    using reference = typename std::array<T,C>::reference;
    using const_reference = typename std::array<T,C>::const_reference;
    using iterator = typename std::array<T,C>::iterator;
    using const_iterator = typename std::array<T,C>::const_iterator;
    using reverse_iterator = typename std::array<T,C>::reverse_iterator;
    using const_reverse_iterator = typename std::array<T,C>::const_reverse_iterator;

// -- ctors --------------------------------------------------------------------
    ///
    /// @brief Default constructor
    /// @details Initializes all elements to zero.
    generic_color() noexcept
    :   channels_()
    {}
    ///
    /// @brief Array constructor
    /// @param a The array to copy.
    constexpr generic_color(const std::array<T,C>& a)
    :   channels_(a)
    {}
    ///
    /// @brief iterator constructor
    /// @details Initializes the color with the elements in the range
    ///          [first, last).
    /// @param first The first element in the range.
    /// @param last The last element in the range.
    /// @tparam InputIt The type of the iterator.
    template<typename InputIt>
    generic_color(InputIt first, InputIt last)
    {   for (std::size_t i = 0; i < C && first != last; ++i)
            channels_[i] = *first++;
    }
    /// @brief copy constructor
    /// @param other The color to copy.
    generic_color(const generic_color& other)
    :   channels_(other.channels_)
    {}
    /// @brief move constructor
    /// @param other The color to move.
    generic_color(generic_color&& other)
    :   channels_(std::move(other.channels_))
    {}
    /// @brief initializer list constructor
    /// @param init The initializer list.
    generic_color(std::initializer_list<value_type> init)
    // :   channels_(init)
    // {}
    {   auto first = init.begin(), last = init.end();
        for (std::size_t i = 0; i < C && first != last; ++i)
            channels_[i] = *first++;
    }
    /// @brief converting constructor from uint32_t
    /// @param rgba The 32-bit RGBA value.
    constexpr generic_color(uint32_t rgba)
    :   channels_
    (   detail::uint32_to_array<sizeof(T), std::array<T, C>>::convert
        (   rgba
        ,   std::is_floating_point<T>()
        )
    )
    {}

// -- element access -------------------------------------------------------------
    ///
    /// @brief Access specified channel with bounds checking
    /// @param pos Position of the channel to return.
    /// @return Reference to the requested channel.
    /// @throws std::out_of_range if `pos >= C`.
    constexpr reference at(size_type pos)
    {   return channels_.at(pos);
    }
    ///
    /// @brief Access specified channel with bounds checking
    /// @param pos Position of the channel to return.
    /// @return Const reference to the requested channel.
    /// @throws std::out_of_range if `pos >= C`.
    constexpr const_reference at(size_type pos) const
    {   return channels_.at(pos);
    }
    ///
    /// @brief Access specified channel
    /// @param pos Position of the channel to return.
    /// @return Reference to the requested channel. No bounds checking is
    ///     performed.
    constexpr reference operator[](size_type pos)
    {   return channels_[pos];
    }
    ///
    /// @brief Access specified channel
    /// @param pos Position of the channel to return.
    /// @return Const reference to the requested channel. No bounds checking
    ///     is performed.
    constexpr const_reference operator[](size_type pos) const
    {   return channels_[pos];
    }
    ///
    /// @brief Access the first channel
    /// @return Reference to the first channel.
    constexpr reference front()
    {   return channels_.front();
    }
    ///
    /// @brief Access the first channel
    /// @return Const reference to the first channel.
    constexpr const_reference front() const
    {   return channels_.front();
    }
    ///
    /// @brief Access the last channel
    /// @return Reference to the last channel.
    constexpr reference back()
    {   return channels_.back();
    }
    ///
    /// @brief Access the last channel
    /// @return Const reference to the last channel.
    constexpr const_reference back() const
    {   return channels_.back();
    }
    ///
    /// @brief Direct access to the underlying array
    /// @return Pointer to the underlying element storage. The pointer is
    ///     such that range `[data(), data() + size())` is always valid.
    constexpr T* data() noexcept
    {   return channels_.data();
    }
    ///
    /// @brief Direct access to the underlying array
    /// @return Const pointer to the underlying element storage. The pointer
    ///     is such that range `[data(), data() + size())` is always valid.
    constexpr const T* data() const noexcept
    {   return channels_.data();
    }

// -- iterators --------------------------------------------------------------
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Iterator to the first channel.
    constexpr iterator begin() noexcept
    {   return channels_.begin();
    }
    ///
    /// @brief Returns an iterator to the beginning
    /// @return Const iterator to the first channel.
    constexpr const_iterator begin() const noexcept
    {   return channels_.begin();
    }
    ///
    /// @brief Returns a const iterator to the beginning
    /// @return Const iterator to the first channel.
    constexpr const_iterator cbegin() const noexcept
    {   return channels_.cbegin();
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Iterator to the channel following the last channel.
    constexpr iterator end() noexcept
    {   return channels_.end();
    }
    ///
    /// @brief Returns an iterator to the end
    /// @return Const iterator to the channel following the last channel.
    constexpr const_iterator end() const noexcept
    {   return channels_.end();
    }
    ///
    /// @brief Returns a const iterator to the end
    /// @return Const iterator to the channel following the last channel.
    constexpr const_iterator cend() const noexcept
    {   return channels_.cend();
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr reverse_iterator rbegin() noexcept
    {   return channels_.rbegin();
    }
    ///
    /// @brief Returns a reverse iterator to the beginning
    /// @return Const reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr const_reverse_iterator rbegin() const noexcept
    {   return channels_.rbegin();
    }
    ///
    /// @brief Returns a const reverse iterator to the beginning
    /// @return Const reverse iterator to the first channel of the reversed
    ///     color (i.e. the last channel).
    constexpr const_reverse_iterator crbegin() const noexcept
    {   return channels_.crbegin();
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Reverse iterator to the channel following the last channel
    ///     of the reversed color (i.e. preceding the first channel).
    constexpr reverse_iterator rend() noexcept
    {   return channels_.rend();
    }
    ///
    /// @brief Returns a reverse iterator to the end
    /// @return Const reverse iterator to the channel following the last
    ///     channel of the reversed color (i.e. preceding the first channel).
    constexpr const_reverse_iterator rend() const noexcept
    {   return channels_.rend();
    }
    ///
    /// @brief Returns a const reverse iterator to the end
    /// @return Const reverse iterator to the channel following the last
    ///     channel of the reversed color (i.e. preceding the first channel).
    constexpr const_reverse_iterator crend() const noexcept
    {   return channels_.crend();
    }

// -- capacity -----------------------------------------------------------------
    ///
    /// @brief Checks whether the color has no channels
    /// @return `true` if `C == 0`, `false` otherwise.
    [[nodiscard]] constexpr bool empty() const noexcept
    {   return channels_.empty();
    }
    ///
    /// @brief Returns the number of channels
    /// @return The number of channels, i.e. `C`.
    constexpr size_type size() const noexcept
    {   return channels_.size();
    }
    ///
    /// @brief Returns the maximum possible number of channels
    /// @return The maximum number of channels, i.e. `C`.
    constexpr size_type max_size() const noexcept
    {   return channels_.max_size();
    }

// -- operations ---------------------------------------------------------------
    ///
    /// @brief Fills the color with the given value
    /// @param value The value to assign to all channels.
    constexpr void fill(const T& value)
    {   channels_.fill(value);
    }
    ///
    /// @brief Swaps the contents with another color
    /// @param other The color to exchange channels with.
    constexpr void swap(generic_color& other)
        noexcept(std::is_nothrow_swappable_v<T>)
    {   channels_.swap(other.channels_);
    }

// -- comparisons ----------------------------------------------------------------
    ///
    /// @brief Equality comparison
    /// @param lhs The left-hand side color.
    /// @param rhs The right-hand side color.
    /// @return `true` if every channel of `lhs` compares equal to the
    ///     corresponding channel of `rhs`, `false` otherwise.
    friend constexpr bool operator==
    (   const generic_color& lhs
    ,   const generic_color& rhs
    )   noexcept
    {   return lhs.channels_ == rhs.channels_;
    }
    ///
    /// @brief Three-way comparison
    /// @param lhs The left-hand side color.
    /// @param rhs The right-hand side color.
    /// @return The lexicographical ordering between the channels of `lhs`
    ///     and `rhs`. Also enables `<`, `<=`, `>` and `>=`.
    friend constexpr auto operator<=>
    (   const generic_color& lhs
    ,   const generic_color& rhs
    )   noexcept
    {   return lhs.channels_ <=> rhs.channels_;
    }

// -- implementation -----------------------------------------------------------
private:
   std::array<T, C> channels_; // The color channels.
};

} // namespace pxl
