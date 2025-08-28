# CLAP Stereo Effect Template CMakeLists.txt
# Builds a CLAP audio plugin using madronalib/mlvg framework

cmake_minimum_required(VERSION 3.10...3.27)

# Set project name and version
project(clap-stereo-effect-template VERSION 0.1.0)

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

# Create embedded font resources
# This embeds the fonts from mlvg examples into our plugin
create_resources(external/mlvg/examples/app/resources build/resources/clap-stereo-effect-template)

# Gather plugin source files
file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src/*.h")

# Create the plugin library
add_library(clap-stereo-effect-template MODULE 
  ${PLUGIN_SOURCES}
  ${PLUGIN_HEADERS}
  build/resources/clap-stereo-effect-template/resources.c
)

# Set the generated resource files to compile as C++
set_source_files_properties(build/resources/clap-stereo-effect-template/resources.c PROPERTIES COMPILE_FLAGS "-x c++")

# Link against madronalib and mlvg
target_link_libraries(clap-stereo-effect-template PRIVATE madronalib mlvg)

# Set up nanovg include directories like mlvg does
set(MLVG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/mlvg/source")
set(NANOVG_INCLUDE_DIRS
    ${MLVG_SOURCE_DIR}/external/nanovg/src
    ${MLVG_SOURCE_DIR}/external/nanosvg/src
    ${MLVG_SOURCE_DIR}/external/MetalNanoVG/src
)

# Add include directories
# We need to explicitly add the include directories since the libraries use global include_directories()
target_include_directories(clap-stereo-effect-template PRIVATE
  external/madronalib/include
  external/madronalib/source
  external/madronalib/source/app
  external/madronalib/source/DSP
  external/madronalib/source/matrix
  external/madronalib/source/networking
  external/madronalib/source/procs
  external/madronalib/external/cJSON
  external/madronalib/external/clap
  external/madronalib/external/ffft
  external/madronalib/external/oscpack
  external/madronalib/external/rtaudio
  external/madronalib/external/rtmidi
  external/madronalib/external/sse2neon
  external/madronalib/external/utf
  external/mlvg/include
  external/mlvg/source
  external/mlvg/source/common
  external/mlvg/source/native
  external/mlvg/source/vg
  external/mlvg/source/widgets
  external/mlvg/source/external
  ${NANOVG_INCLUDE_DIRS}
  src
)

# Enable GUI support
target_compile_definitions(clap-stereo-effect-template PRIVATE HAS_GUI=1)

# Platform-specific configuration
if(APPLE)
  set_target_properties(clap-stereo-effect-template PROPERTIES
    BUNDLE True
    BUNDLE_EXTENSION clap
    MACOSX_BUNDLE_GUI_IDENTIFIER com.madronalabs.clap-stereo-effect-template
    MACOSX_BUNDLE_BUNDLE_NAME "Clap Stereo Effect Template"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/clap-stereo-effect-template.plist.in
  )
  
  # Link macOS frameworks
  target_link_libraries(clap-stereo-effect-template PRIVATE 
    "-framework CoreFoundation" 
    "-framework AppKit" 
    "-framework CoreGraphics"
  )
  
  # Add Metal support for GUI
  find_library(METAL_FRAMEWORK Metal)
  find_library(METALKIT_FRAMEWORK MetalKit)
  if(METAL_FRAMEWORK AND METALKIT_FRAMEWORK)
    target_link_libraries(clap-stereo-effect-template PRIVATE ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
  endif()
  
  target_compile_definitions(clap-stereo-effect-template PRIVATE IS_MAC=1)
  
elseif(UNIX)
  target_compile_definitions(clap-stereo-effect-template PRIVATE IS_LINUX=1)
  set_target_properties(clap-stereo-effect-template PROPERTIES SUFFIX ".clap" PREFIX "")
  
elseif(WIN32)
  target_compile_definitions(clap-stereo-effect-template PRIVATE IS_WIN=1)
  set_target_properties(clap-stereo-effect-template PROPERTIES SUFFIX ".clap" PREFIX "")
endif()

# Set output directory
set_target_properties(clap-stereo-effect-template PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clap
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clap
)

# Print build information
message(STATUS "Building CLAP Stereo Effect Template")
message(STATUS "Target: clap-stereo-effect-template")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/clap")
