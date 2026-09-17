// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Armin Sobhani
//
#pragma once

#include <pxl/detail/widgets.hpp>

#ifdef __CLING__

namespace pxl {

// -- color_widget -------------------------------------------------------------

template <typename T, std::size_t C>
struct color_widget
{   static_assert
    (   std::is_integral<T>()
    ||  std::is_floating_point<T>()
    ,   "color_widget: T must be an integral or floating point type"
    );
    static_assert
    (   C > 0
    ,   "color_widget: C must be greater than 0"
    );
    using value_type = T;
    using color_type = generic_color<T, C>;

    color_widget(color_type& color)
    :   color_(color)
    {   picker_.value
    =   detail::color_to_hex(color_, std::is_floating_point<T>()).data();
        XOBSERVE(picker_, value, [this](const auto& owner)
        {   auto value = owner.value();
            value += "FF"; // add alpha channel
            uint32_t rgba = std::stoul(value.substr(1), nullptr, 16);
            color_ = color_type(rgba);
        });
    }
    void display() const
    {   picker_.display();
    }

private:
    color_type& color_;
    xw::color_picker picker_;
};

template <typename T, std::size_t C>
color_widget<T, C> widget(generic_color<T, C>& color)
{   return color_widget<T, C>(color);
}

template <typename T, std::size_t C>
nlohmann::json mime_bundle_repr(generic_color<T, C> const& color)
{   auto bundle = nlohmann::json::object();
    std::stringstream ss;
    image_no_init<T, C> image(21, 21, 0x777777ff);
    auto view = image(1, 1, 19, 19);
    std::fill(view.begin(), view.end(), color);
    image.save(ss, io::out::png());
    bundle["image/png"] = xtl::base64encode(ss.str());
    return bundle;
}

// -- image_widget -------------------------------------------------------------

template <Pixel PixelType, typename Container>
struct image_widget
{
    // static_assert
    // (   std::contiguous_iterator<typename Container::iterator>()
    // ,   "image_widget: Container must be a contiguous iterator"
    // );
    using image_type = generic_image<PixelType, Container>;

    image_widget(image_type& image)
    :   image_(image)
    ,   widget_()
    {   auto temp_path = std::filesystem::temp_directory_path() / "pxl_image.png";
        image_.save(temp_path.string(), io::out::png());
        auto w = xw::image_from_file(temp_path.string());
        std::filesystem::remove(temp_path);
        widget_ = w; 
    }
    void display() const
    {   widget_.display();
    }

private:
    image_type& image_;
    xw::image widget_;
};

template <Pixel PixelType, typename Container>
image_widget<PixelType, Container> widget(generic_image<PixelType, Container>& image)
{   return image_widget<PixelType, Container>(image);
}

template <Pixel PixelType, typename Container>
nlohmann::json mime_bundle_repr
(   generic_image<PixelType, Container> const& image
)
{   auto bundle = nlohmann::json::object();
    std::stringstream ss;
    image.save(ss, io::out::png());
    bundle["image/png"] = xtl::base64encode(ss.str());
    return bundle;
}

// -- image_view_widget --------------------------------------------------------

template <Pixel PixelType>
struct image_view_widget
{
    using view_type = generic_image_view<PixelType>;

    image_view_widget(view_type& view)
    :   view_(view)
    ,   widget_()
    {   image_no_init
        <   typename PixelType::value_type
        ,   PixelType::max_size()
        >   image(view);
        auto temp_path = std::filesystem::temp_directory_path() / "pxl_view.png";
        image.save(temp_path.string(), io::out::png());
        auto w = xw::image_from_file(temp_path.string());
        std::filesystem::remove(temp_path);
        widget_ = w;
    }
    void display() const
    {   widget_.display();
    }

private:
    view_type& view_;
    xw::image widget_;
};

template <Pixel PixelType>
image_view_widget<PixelType> widget(generic_image_view<PixelType>& view)
{   return image_view_widget<PixelType>(view);
}

template <Pixel PixelType>
nlohmann::json mime_bundle_repr
(   generic_image_view<PixelType> const& view
)
{   auto bundle = nlohmann::json::object();
    std::stringstream ss;
    image_no_init
    <   typename PixelType::value_type
    ,   PixelType::max_size()
    >   image(view);
    image.save(ss, io::out::png());
    bundle["image/png"] = xtl::base64encode(ss.str());
    return bundle;
}

} // end pxl namespace

#endif // __CLING__