# DSP Unit Tests Configuration
# This file contains the configuration for building and running DSP component unit tests

# Option to build unit tests for our DSP components
option(BUILD_DSP_TESTS "Build unit tests for DSP components" OFF)

if(BUILD_DSP_TESTS)
  enable_testing()
  
  # Create test executable for TimeVaryingWavetableGen
  add_executable(wavetable_tests
    src/dsp/MLDSPWavetableGenTest.cpp
  )
  
  # Include directories for tests (match madronalib target includes)
  target_include_directories(wavetable_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/dsp
    ${CMAKE_SOURCE_DIR}/external/madronalib/source/DSP
    ${CMAKE_SOURCE_DIR}/external/madronalib/source/matrix
    ${CMAKE_SOURCE_DIR}/external/madronalib/source/app
    ${CMAKE_SOURCE_DIR}/external/madronalib/Tests
    ${CMAKE_SOURCE_DIR}/external/madronalib/external
    ${CMAKE_SOURCE_DIR}/external/madronalib/external/sse2neon
    ${CMAKE_SOURCE_DIR}/external/madronalib/include
  )
  
  # Link against madronalib
  target_link_libraries(wavetable_tests madronalib)
  
  # Set C++ standard to match main project
  target_compile_features(wavetable_tests PRIVATE cxx_std_17)
  
  # Copy compiler definitions from madronalib target
  get_target_property(MADRONALIB_COMPILE_DEFINITIONS madronalib COMPILE_DEFINITIONS)
  if(MADRONALIB_COMPILE_DEFINITIONS)
    target_compile_definitions(wavetable_tests PRIVATE ${MADRONALIB_COMPILE_DEFINITIONS})
  endif()
  
  # Add compiler flags for better debugging
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(wavetable_tests PRIVATE 
      $<$<CXX_COMPILER_ID:GNU,Clang>:-g -O0>
      $<$<CXX_COMPILER_ID:MSVC>:/Od /Zi>
    )
  endif()
  
  # Add the test to CTest
  add_test(NAME TimeVaryingWavetableGenTests COMMAND wavetable_tests)
  
  # Custom target to run tests easily
  add_custom_target(run_dsp_tests
    COMMAND wavetable_tests
    DEPENDS wavetable_tests
    COMMENT "Running DSP unit tests"
    VERBATIM
  )
  
  message(STATUS "DSP unit tests enabled. Use 'make run_dsp_tests' to run them.")
endif()

