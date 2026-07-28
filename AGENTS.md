# pxl

C++20 header-only image processing library. See [README.md](README.md) for the one-line project description.

## Build / configure / test

Requires the `VCPKG_INSTALLATION_ROOT` environment variable to point to a vcpkg checkout (dependencies: `pthreads`, and `catch2` via the `tests` manifest feature).

```bash
cmake -S . --preset vcpkg -B build   # configure (Ninja generator, picks up vcpkg toolchain)
cmake --build build                  # build
ctest --test-dir build --output-on-failure   # run unit tests
```

- In-source builds are rejected by CMake — always build into `build/`.
- `find_package(OpenMP REQUIRED)` — the toolchain must have OpenMP available (on macOS this needs `brew install libomp` and `OpenMP_ROOT` set; see [.github/workflows/cmake-multi-platform.yml](.github/workflows/cmake-multi-platform.yml)).
- Key CMake options (see [CMakeLists.txt](CMakeLists.txt)): `PXL_ENABLE_TESTS` (default ON), `PXL_ENABLE_CUDA`, `PXL_ENABLE_HIP`.
- CI (`.github/workflows/cmake-multi-platform.yml`) builds/tests on Linux, Windows, and macOS using the `vcpkg` preset on every PR to `main`.

## Architecture

- Header-only library: `include/pxl/*.hpp` is the public API (e.g. [color.hpp](include/pxl/color.hpp)); `include/pxl/detail/*.hpp` holds implementation details (e.g. [detail/color.hpp](include/pxl/detail/color.hpp)) not meant to be included directly by users.
- The `pxl` CMake target is an `INTERFACE` library; installation of headers/CMake package files is skipped automatically when pxl is consumed as a subproject (`NOT_SUBPROJECT` check in [CMakeLists.txt](CMakeLists.txt)).

## Conventions

- Every source file starts with an SPDX license header comment: `// SPDX-License-Identifier: MIT` and `// Copyright (c) <year> Armin Sobhani`.
- Public API is documented with Doxygen-style `///` comments (`@brief`, `@param`, `@tparam`, `@return`, `@throws`) — follow the existing style in [color.hpp](include/pxl/color.hpp) when adding members.
- Code style uses a distinctive brace/alignment convention (opening brace on its own indented line, continuation lines aligned with `:   ` / `{   ` before the statement) — match the surrounding code rather than reformatting to a different style.
- Prefer `consteval`/`constexpr` where the existing code already does (e.g. conversion helpers in [detail/color.hpp](include/pxl/detail/color.hpp)).

## Tests

- Framework: Catch2 v3, one `test_<name>.cpp` file per component under [test/](test/).
- Register new test files by adding a call to the `add_unit_tests(<name> <source>.cpp)` function followed by `catch_discover_tests(<target> TEST_PREFIX "[Core] ")` in [test/CMakeLists.txt](test/CMakeLists.txt) — this handles CUDA/HIP source-language switching automatically.
- `TEST_CASE` names are full descriptive sentences; tags are lowercase and hierarchical, e.g. `"[color][ctors]"`, `"[color][element_access]"` — group related cases under `// -- section --` comment banners as in [test_color_class.cpp](test/test_color_class.cpp).
