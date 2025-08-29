# CLAP Stereo Effect Template CMakeLists.txt
# Builds a CLAP audio plugin using madronalib/mlvg framework

cmake_minimum_required(VERSION 3.10...3.27)

# Parse plugin metadata to get project name and version
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}/scripts
  python3 ${CMAKE_SOURCE_DIR}/scripts/parse_metadata.py ${CMAKE_SOURCE_DIR}/plugin-metadata.json
  OUTPUT_VARIABLE METADATA_CMAKELISTS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Include the generated CMake variables
file(WRITE ${CMAKE_BINARY_DIR}/plugin_metadata.cmake "${METADATA_CMAKELISTS}")
include(${CMAKE_BINARY_DIR}/plugin_metadata.cmake)

# Set project name and version from metadata
project(${PLUGIN_PROJECT_NAME} VERSION ${PLUGIN_VERSION})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)

# Platform-specific settings
if(APPLE)
  set(CMAKE_OSX_ARCHISTECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-aligned-new")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4068")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:alignedNew-")
  
  if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    add_compile_options(
      $<$<CONFIG:>:/MT>
      $<$<CONFIG:Debug>:/MTd>
      $<$<CONFIG:Release>:/MT>
    )
  endif()
endif()

# Set module path for custom CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Add subdirectories for external libraries
# Disable tests in both libraries to avoid conflicts
set(BUILD_TESTS OFF CACHE BOOL "Build the tests" FORCE)
add_subdirectory(external/madronalib EXCLUDE_FROM_ALL)
add_subdirectory(external/mlvg EXCLUDE_FROM_ALL)

# Include the CLAP plugin abstraction
include(CLAPPlugin)

# Include font embedding utilities
include(FontEmbedding)

# Include CLAP development tools (optional)
option(BUILD_CLAP_TOOLS "Build CLAP development tools" ON)
if(BUILD_CLAP_TOOLS)
  include(CLAPTools)
endif()

# Create the CLAP plugin
create_clap_plugin(${PLUGIN_PROJECT_NAME})

# Add custom fonts
add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")


