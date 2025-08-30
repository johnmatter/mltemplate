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

# Enable Objective-C and Objective-C++ languages for clap-wrapper on Apple
if(APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

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
  
  # Add custom target to build VST3 test host
  add_custom_target(vst3-testhost
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/build-vst3-testhost.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building VST3 Plugin Test Host..."
    VERBATIM
  )
endif()

# Add custom fonts
add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")

# Add custom install target for development
add_custom_target(install-clap
  COMMAND ${CMAKE_COMMAND} --install .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Installing CLAP plugin to system directories..."
  VERBATIM
)

# Add custom target to install CLAP plugin to user directory only (no admin required)
add_custom_target(install-clap-user
  COMMAND ${CMAKE_COMMAND} --install . --prefix $ENV{HOME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Installing CLAP plugin to user directory..."
  VERBATIM
)


