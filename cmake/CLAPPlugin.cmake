# CLAPPlugin.cmake - Reusable CMake module for CLAP plugins
# Usage: include(CLAPPlugin)
# Note: Requires PLUGIN_PROJECT_NAME and other metadata variables to be set

# Function to create a CLAP plugin target
function(create_clap_plugin TARGET_NAME)
  # Generate CLAP entry point from metadata
  execute_process(
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/generate_entry_point.py ${CMAKE_SOURCE_DIR}/plugin-metadata.json
    OUTPUT_VARIABLE ENTRY_POINT_CONTENT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Write the generated entry point
  file(WRITE ${CMAKE_BINARY_DIR}/generated_entry_point.cpp "${ENTRY_POINT_CONTENT}")

  # Create embedded font resources
  create_resources(external/mlvg/examples/app/resources build/resources/${TARGET_NAME})

  # Gather plugin source files (excluding the entry point which will be generated)
  file(GLOB PLUGIN_SOURCES "src/*.cpp")
  list(REMOVE_ITEM PLUGIN_SOURCES "${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}-entry.cpp")
  file(GLOB PLUGIN_HEADERS "src/*.h")

  # Create the plugin library
  add_library(${TARGET_NAME} MODULE 
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    ${CMAKE_BINARY_DIR}/generated_entry_point.cpp
    build/resources/${TARGET_NAME}/resources.c
  )

  # Set the generated resource files to compile as C++
  set_source_files_properties(build/resources/${TARGET_NAME}/resources.c PROPERTIES COMPILE_FLAGS "-x c++")

  # Link against madronalib and mlvg
  target_link_libraries(${TARGET_NAME} PRIVATE madronalib mlvg)

  # Set up nanovg include directories
  set(MLVG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/mlvg/source")
  set(NANOVG_INCLUDE_DIRS
      ${MLVG_SOURCE_DIR}/external/nanovg/src
      ${MLVG_SOURCE_DIR}/external/nanosvg/src
      ${MLVG_SOURCE_DIR}/external/MetalNanoVG/src
  )

  # Add include directories
  target_include_directories(${TARGET_NAME} PRIVATE
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
  target_compile_definitions(${TARGET_NAME} PRIVATE HAS_GUI=1)

  # Platform-specific configuration
  if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES
      BUNDLE True
      BUNDLE_EXTENSION clap
      MACOSX_BUNDLE_GUI_IDENTIFIER ${PLUGIN_BUNDLE_IDENTIFIER}
      MACOSX_BUNDLE_BUNDLE_NAME "${PLUGIN_BUNDLE_NAME}"
      MACOSX_BUNDLE_BUNDLE_VERSION "${PLUGIN_BUNDLE_VERSION}"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PLUGIN_BUNDLE_SHORT_VERSION}"
      MACOSX_BUNDLE_LONG_VERSION_STRING "${PLUGIN_BUNDLE_LONG_VERSION}"
      MACOSX_BUNDLE_INFO_STRING "${PLUGIN_BUNDLE_INFO_STRING}"
      MACOSX_BUNDLE_COPYRIGHT "${PLUGIN_BUNDLE_COPYRIGHT}"
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/${TARGET_NAME}.plist.in
    )
    
    # Link macOS frameworks
    target_link_libraries(${TARGET_NAME} PRIVATE 
      "-framework CoreFoundation" 
      "-framework AppKit" 
      "-framework CoreGraphics"
    )
    
    # Add Metal support for GUI
    find_library(METAL_FRAMEWORK Metal)
    find_library(METALKIT_FRAMEWORK MetalKit)
    if(METAL_FRAMEWORK AND METALKIT_FRAMEWORK)
      target_link_libraries(${TARGET_NAME} PRIVATE ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
    endif()
    
    target_compile_definitions(${TARGET_NAME} PRIVATE IS_MAC=1)
    
  elseif(UNIX)
    target_compile_definitions(${TARGET_NAME} PRIVATE IS_LINUX=1)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
    
  elseif(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE IS_WIN=1)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
  endif()

  # Set output directory
  set_target_properties(${TARGET_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clap
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clap
  )

  # Install target
  include(GNUInstallDirs)
  if(APPLE)
    set(CLAP_INSTALL_DIR "~/Library/Audio/Plug-Ins/CLAP")
  elseif(UNIX)
    set(CLAP_INSTALL_DIR "~/.clap")
  elseif(WIN32)
    set(CLAP_INSTALL_DIR "$ENV{APPDATA}/CLAP")
  endif()

  install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CLAP_INSTALL_DIR}
    RUNTIME DESTINATION ${CLAP_INSTALL_DIR}
    BUNDLE DESTINATION ${CLAP_INSTALL_DIR}
  )

  # Print build information
  message(STATUS "Plugin: ${PLUGIN_NAME} v${PLUGIN_VERSION}")
  message(STATUS "Creator: ${PLUGIN_CREATOR}")
  message(STATUS "CLAP ID: ${PLUGIN_CLAP_ID}")
  message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/clap")
  message(STATUS "Install directory: ${CLAP_INSTALL_DIR}")
  message(STATUS "Run `make install` to install to system CLAP plugin directories.")
endfunction()
