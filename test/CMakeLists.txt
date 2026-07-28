# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Armin Sobhani
#

## function to add unit tests
#
function(add_unit_tests TEST_NAME TEST_SOURCE)
  get_filename_component(TEST_TARGET ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_TARGET} ${TEST_SOURCE})
  target_link_libraries(${TEST_TARGET} PRIVATE
    ${PROJECT_NAME}::${PROJECT_NAME}
    Catch2::Catch2WithMain
  )
  if(CUDAToolkit_FOUND)
    set_source_files_properties(${TEST_SOURCE} PROPERTIES LANGUAGE CUDA)
    target_compile_options(${TEST_TARGET} PRIVATE
      $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
      $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CUDA>>:-G;-src-in-ptx>
    )
  endif()
  if(hip_FOUND)
    set_source_files_properties(${TEST_SOURCE} PROPERTIES LANGUAGE HIP)
    target_compile_options(${TEST_TARGET} PRIVATE
      -Wno-unused-result -Wno-pass-failed
      $<$<COMPILE_LANGUAGE:HIP>:--offload-arch=gfx942>
    )
  endif()
endfunction()

## defining unit tests
#
add_unit_tests("color_class" test_color_class.cpp)

## finally adding unit tests
#
catch_discover_tests(test_color_class TEST_PREFIX "[Core] ")