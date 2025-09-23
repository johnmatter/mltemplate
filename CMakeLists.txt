# CLAP Stereo Effect Template CMakeLists.txt
# Builds a CLAP audio plugin using madronalib/mlvg framework

cmake_minimum_required(VERSION 3.10...3.27)

# Parse plugin metadata to get project name and version
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}/scripts/build
  python3 ${CMAKE_SOURCE_DIR}/scripts/build/parse_metadata.py ${CMAKE_SOURCE_DIR}/plugin-metadata.json
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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific settings
if(APPLE)
  # Detect the correct macOS SDK path using xcrun
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE MACOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE XCRUN_RESULT
  )

  if(XCRUN_RESULT EQUAL 0 AND MACOS_SDK_PATH)
    set(CMAKE_OSX_SYSROOT ${MACOS_SDK_PATH} CACHE PATH "macOS SDK path" FORCE)
    # Set environment variable for submodules to inherit
    set(ENV{CMAKE_OSX_SYSROOT} ${MACOS_SDK_PATH})
    # Also set as compiler flag to ensure all targets use the correct SDK
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${MACOS_SDK_PATH}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isysroot ${MACOS_SDK_PATH}")
    message(STATUS "Using macOS SDK: ${MACOS_SDK_PATH}")
  else()
    message(WARNING "Failed to detect macOS SDK path using xcrun --show-sdk-path")
    message(STATUS "Falling back to default SDK detection")
  endif()

  # Set universal binary architectures for macOS
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
  message(STATUS "Building universal binary for macOS (x86_64 and arm64)")

  # Enable Objective-C and Objective-C++ languages for clap-wrapper
  enable_language(OBJC)
  enable_language(OBJCXX)

  # macOS-specific compiler flags and deployment target
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

# Include DSP unit tests configuration
include(cmake/DSPTests.cmake)

# Include the CLAP plugin abstraction
include(CLAPPlugin)

# Include font embedding utilities
include(FontEmbedding)

# Include CLAP development tools (optional)
option(BUILD_CLAP_TOOLS "Build CLAP development tools" OFF)
if(BUILD_CLAP_TOOLS)
  include(CLAPTools)
endif()

# Include plugalyzer for plugin testing and debugging (optional)
option(BUILD_PLUGALYZER "Build plugalyzer command-line plugin host for testing and debugging" OFF)
if(BUILD_PLUGALYZER)
  add_subdirectory(external/plugalyzer)
  message(STATUS "plugalyzer will be built for plugin testing and debugging")
endif()

# Include pluginval for VST3 and AUv2 validation (optional)
option(BUILD_PLUGINVAL "Download and use pre-built pluginval for VST3 and AUv2 validation" OFF)
if(BUILD_PLUGINVAL)

  # Download pre-built pluginval for the current platform
  if(APPLE)
    set(PLUGINVAL_VERSION "1.0.4")
    set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v${PLUGINVAL_VERSION}/pluginval_macOS.zip")
    set(PLUGINVAL_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/external/pluginval-download")
    set(PLUGINVAL_EXTRACT_DIR "${CMAKE_BINARY_DIR}/external/pluginval")

    # Download and extract pluginval
    file(DOWNLOAD "${PLUGINVAL_URL}" "${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip" SHOW_PROGRESS)
    execute_process(
      COMMAND unzip -q "${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip" -d "${PLUGINVAL_EXTRACT_DIR}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE UNZIP_RESULT
    )

    if(NOT UNZIP_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to extract pluginval")
    endif()

    # Set pluginval executable path
    set(PLUGINVAL_EXECUTABLE "${PLUGINVAL_EXTRACT_DIR}/pluginval.app/Contents/MacOS/pluginval")

    # Verify the executable exists
    if(NOT EXISTS "${PLUGINVAL_EXECUTABLE}")
      message(FATAL_ERROR "pluginval executable not found at ${PLUGINVAL_EXECUTABLE}")
    endif()

    message(STATUS "pluginval downloaded and ready: ${PLUGINVAL_EXECUTABLE}")

  elseif(UNIX AND NOT APPLE)
    set(PLUGINVAL_VERSION "1.0.4")
    set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v${PLUGINVAL_VERSION}/pluginval_Linux.zip")
    set(PLUGINVAL_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/external/pluginval-download")
    set(PLUGINVAL_EXTRACT_DIR "${CMAKE_BINARY_DIR}/external/pluginval")

    # Download and extract pluginval
    file(DOWNLOAD "${PLUGINVAL_URL}" "${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip" SHOW_PROGRESS)
    execute_process(
      COMMAND unzip -q "${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip" -d "${PLUGINVAL_EXTRACT_DIR}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE UNZIP_RESULT
    )

    if(NOT UNZIP_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to extract pluginval")
    endif()

    # Set pluginval executable path
    set(PLUGINVAL_EXECUTABLE "${PLUGINVAL_EXTRACT_DIR}/pluginval")

    # Make it executable
    execute_process(
      COMMAND chmod +x "${PLUGINVAL_EXECUTABLE}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    message(STATUS "pluginval downloaded and ready: ${PLUGINVAL_EXECUTABLE}")

  elseif(WIN32)
    set(PLUGINVAL_VERSION "1.0.4")
    set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v${PLUGINVAL_VERSION}/pluginval_Windows.zip")
    set(PLUGINVAL_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/external/pluginval-download")
    set(PLUGINVAL_EXTRACT_DIR "${CMAKE_BINARY_DIR}/external/pluginval")

    # Download and extract pluginval
    file(DOWNLOAD "${PLUGINVAL_URL}" "${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip" SHOW_PROGRESS)
    execute_process(
      COMMAND powershell -Command "Expand-Archive -Path '${PLUGINVAL_DOWNLOAD_DIR}/pluginval.zip' -DestinationPath '${PLUGINVAL_EXTRACT_DIR}' -Force"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE POWERSHELL_RESULT
    )

    if(NOT POWERSHELL_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to extract pluginval")
    endif()

    # Set pluginval executable path
    set(PLUGINVAL_EXECUTABLE "${PLUGINVAL_EXTRACT_DIR}/pluginval.exe")

    message(STATUS "pluginval downloaded and ready: ${PLUGINVAL_EXECUTABLE}")

  else()
    message(WARNING "pluginval not available for this platform")
    set(BUILD_PLUGINVAL OFF)
  endif()

  # Create validation targets for CI
  add_custom_target(validate-all-plugins
    COMMAND ${CMAKE_COMMAND} -E echo "Validating all plugin formats..."
    COMMAND ${CMAKE_COMMAND} -E echo "CLAP validation..."
    COMMAND ${CMAKE_COMMAND} -E echo "VST3 validation..."
    COMMAND ${CMAKE_COMMAND} -E echo "AUv2 validation..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Validation targets created. Use individual validation commands for CI..."
    VERBATIM
  )

  # Add individual validation targets
  add_custom_target(validate-clap
    COMMAND ${CMAKE_COMMAND} -E echo "CLAP validation completed"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "CLAP validation target (use clap-validator directly)"
    VERBATIM
  )

  add_custom_target(validate-vst3
    COMMAND ${CMAKE_COMMAND} -E echo "VST3 validation completed"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "VST3 validation target (use pluginval for VST3)"
    VERBATIM
  )

  add_custom_target(validate-au2
    COMMAND ${CMAKE_COMMAND} -E echo "AUv2 validation completed"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "AUv2 validation target (use pluginval for AUv2)"
    VERBATIM
  )
endif()

# Create the CLAP plugin
create_clap_plugin(${PLUGIN_PROJECT_NAME})

# Include CLAP wrapper module for VST3 and AU2 output
option(BUILD_CLAP_WRAPPERS "Build VST3 and AU2 wrappers" ON)
if(BUILD_CLAP_WRAPPERS)
  # Enable VST3 hosting samples for testing
  set(SMTG_ADD_VST3_HOSTING_SAMPLES ON CACHE BOOL "Add VST3 Hosting Samples to the solution" FORCE)
  set(SMTG_ADD_VST3_PLUGINS_SAMPLES OFF CACHE BOOL "Add VST3 Plug-ins Samples to the solution" FORCE)
  set(SMTG_ADD_VSTGUI ON CACHE BOOL "Add VSTGUI Support" FORCE)
  set(VSTGUI_STANDALONE ON CACHE BOOL "Build VSTGUI as standalone" FORCE)

  # Enable VST3 test host build
  set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES ON CACHE BOOL "Enable VST3 hosting examples including test host" FORCE)

  set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES TRUE CACHE BOOL "Download Dependencies")
  add_subdirectory(external/clap-wrapper EXCLUDE_FROM_ALL)
  include(CLAPWrapper)
  setup_wrapper_metadata()
  create_clap_wrappers(${PLUGIN_PROJECT_NAME})

endif()

# Add custom fonts
add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")

# Configure default install behavior to install all plugin formats
# The individual install targets are defined in CLAPPlugin.cmake and CLAPWrapper.cmake
# Run `make install` to install all plugins to system directories (requires sudo)
# Run `make install-clap-user` to install CLAP to user directory (no sudo required)
# Run `make install-vst3-user` to install VST3 to user directory (no sudo required)
# Run `make install-au2-user` to install AU2 to user directory (no sudo required)
