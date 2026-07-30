# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Armin Sobhani
#
cmake_minimum_required(VERSION 3.25.2...4.4.0)

## detect if gnx is being bundled, disable unit tests and benchmarks in that case
#
if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

## life is about choices...
#
option(PXL_ENABLE_TESTS "Enable the unit tests ?" ON)
option(PXL_ENABLE_CUDA "Enable CUDA support ?" OFF)
option(PXL_ENABLE_HIP "Enable ROCm support ?" OFF)

## vcpkg manifest features
#
if(${PXL_ENABLE_TESTS})
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

## finally our project...
#
project(
  pxl
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "C++20 header-only image processing library"
  HOMEPAGE_URL "https://github.com/arminms/pxl"
)

## prevent in-source builds
#
if (CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

## necessary modules
#
include(GNUInstallDirs)
include(FetchContent)
include(CheckLanguage)

## check for compiler support for OpenMP
#
find_package(OpenMP REQUIRED)

## check for CUDA support
#
if (${GNX_ENABLE_CUDA})
  check_language(CUDA 12)
  if(CMAKE_CUDA_COMPILER)
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      set(CMAKE_CUDA_ARCHITECTURES native) # set to all-major for distribution
    endif()
    enable_language(CUDA)
    find_package(CUDAToolkit)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

## check for ROCm support
#
if (${GNX_ENABLE_HIP})
  check_language(HIP 6)
  if (CMAKE_HIP_COMPILER)
    if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
      set(CMAKE_HIP_ARCHITECTURES native) # set to all-major for distribution
    endif()
    enable_language(HIP)
    find_package(hip HINTS $ENV{HIP_PATH} /opt/rocm/hip /opt/rocm)
    set(CMAKE_HIP_FLAGS "${CMAKE_HIP_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

## check for PThreads
#
if (MSVC)
  find_package(pthreads4w CONFIG REQUIRED)
else()
  find_package(Threads REQUIRED)
endif()

## defining PXL target
#
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
target_link_libraries(${PROJECT_NAME} INTERFACE
  $<IF:$<BOOL:${MSVC}>,PThreads4W::PThreads4W,Threads::Threads>
  OpenMP::OpenMP_CXX
)
if(MSVC)
  target_compile_options(${PROJECT_NAME} INTERFACE
    /openmp:llvm -openmp:experimental)
endif()
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${pxl_VERSION}
  SOVERSION ${pxl_VERSION_MAJOR}
)

## only perform the installation when PXL is not being used as a subproject
#
if(NOT_SUBPROJECT)
  ## specifying the export name for the targets
  #
  install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
  )

  ## installing PXL header files
  #
  add_subdirectory(include)

  ## installing PXL export target file
  #
  install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  ## generate the config file that includes the exports
  #
  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  ## creating version file
  #
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION "${pxl_VERSION_MAJOR}.${pxl_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
  )

  ## installing PXL export config files
  #
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  ## make export target available from the build tree
  #
  export(EXPORT ${PROJECT_NAME}-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
  )

  ## add unit tests
  #
  if(${PXL_ENABLE_TESTS})
    enable_testing()
    find_package(
      Catch2 3
      HINTS $ENV{HOME} /usr/local /opt/local /opt
    )
    if(NOT Catch2_FOUND)
        message(STATUS "Fetching Catch2 library...")
        FetchContent_Declare(
          catch2
          GIT_REPOSITORY https://github.com/catchorg/Catch2.git
          GIT_TAG        v3.5.2
        )
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
    include(Catch)
    add_subdirectory(test)
  endif()

endif(NOT_SUBPROJECT)
