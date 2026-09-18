---
title: PXL Sandbox
description: Start exploring PXL in a sandbox.
kernelspec:
  name: xcpp20-openmp
  display_name: C++20-OpenMP
---

# PXL Sandbox

---

All you have to do to start working with the **PXL** library is to include *the base* header file. Optionally, you can switch to `pxl` namespace as well:

```{code-cell} cpp
#include <pxl/image.hpp>
#include <pxl/widgets.hpp>

using namespace pxl;
```
+++
```{code-cell} cpp
!wget -O test.jpg https://thumb.wikimedia.org/wikipedia/commons/thumb/e/e7/Handyaufnahme_des_Sonnenuntergangs_als_Zeitraffer_20200909_DSC3277.jpg/330px-Handyaufnahme_des_Sonnenuntergangs_als_Zeitraffer_20200909_DSC3277.jpg
```
+++
```{code-cell} cpp
image<> test_image("test.jpg");
test_image
```
+++
Getting a *non-owning*, *zero-copy* view of the image (like `std::string_view`):
```{code-cell} cpp
test_image(104, 57, 127, 71)
```
+++
A view of the specified row in the image:
```{code-cell} cpp
test_image[57]
```
+++
Pixel at x = 104, y = 57 (notice: x <=> y are swapped)
```{code-cell} cpp
test_image[57][104]
```
+++
Alpha channel at pixel 104, 57:
```{code-cell} cpp
test_image[57][104][3]
```
+++
Turning the pixel into a widget:
```{code-cell} cpp
auto color_picker = pxl::widget(test_image[57][104]);
color_picker.display();
```
+++
```{code-cell} cpp
test_image[57]
```
+++
```{code-cell} cpp
test_image
```

