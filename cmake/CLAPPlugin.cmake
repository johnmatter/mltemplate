# CLAPPlugin.cmake - Reusable CMake module for CLAP plugins
# Usage: include(CLAPPlugin)
# Note: Requires PLUGIN_PROJECT_NAME and other metadata variables to be set

# Function to create a CLAP plugin target
function(create_clap_plugin TARGET_NAME)
  # Generate CLAP entry point from metadata
  execute_process(
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/build/generate_entry_point.py ${CMAKE_SOURCE_DIR}/plugin-metadata.json
    OUTPUT_VARIABLE ENTRY_POINT_CONTENT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Write the generated entry point
  file(WRITE ${CMAKE_BINARY_DIR}/generated_entry_point.cpp "${ENTRY_POINT_CONTENT}")

  # Generate plist template in build directory to avoid committing auto-generated files
  set(PLIST_TEMPLATE_PATH "${CMAKE_BINARY_DIR}/cmake/${TARGET_NAME}.plist.in")
  if(NOT EXISTS "${PLIST_TEMPLATE_PATH}")
    message(STATUS "Generating plist template: ${PLIST_TEMPLATE_PATH}")
    
    # Ensure the build/cmake directory exists
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cmake")
    
    # Read the template plist content
    file(READ "${CMAKE_SOURCE_DIR}/cmake/clap-stereo-effect-template.plist.in" PLIST_TEMPLATE_CONTENT)
    
    # Replace the old project name with the new one
    string(REPLACE "clap-stereo-effect-template" "${TARGET_NAME}" PLIST_TEMPLATE_CONTENT "${PLIST_TEMPLATE_CONTENT}")
    
    # Write the new plist template to build directory
    file(WRITE "${PLIST_TEMPLATE_PATH}" "${PLIST_TEMPLATE_CONTENT}")
  endif()

  # Automatically rename header files if they still have the old template name
  set(OLD_HEADER_MAIN "${CMAKE_SOURCE_DIR}/src/clap-stereo-effect-template.h")
  set(NEW_HEADER_MAIN "${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}.h")
  set(OLD_HEADER_GUI "${CMAKE_SOURCE_DIR}/src/clap-stereo-effect-template-gui.h")
  set(NEW_HEADER_GUI "${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}-gui.h")
  
  if(EXISTS "${OLD_HEADER_MAIN}" AND NOT EXISTS "${NEW_HEADER_MAIN}")
    message(STATUS "Renaming header file: ${OLD_HEADER_MAIN} -> ${NEW_HEADER_MAIN}")
    file(RENAME "${OLD_HEADER_MAIN}" "${NEW_HEADER_MAIN}")
  endif()
  
  if(EXISTS "${OLD_HEADER_GUI}" AND NOT EXISTS "${NEW_HEADER_GUI}")
    message(STATUS "Renaming GUI header file: ${OLD_HEADER_GUI} -> ${NEW_HEADER_GUI}")
    file(RENAME "${OLD_HEADER_GUI}" "${NEW_HEADER_GUI}")
  endif()

  # Automatically rename source files if they still have the old template name
  set(OLD_SOURCE_MAIN "${CMAKE_SOURCE_DIR}/src/clap-stereo-effect-template.cpp")
  set(NEW_SOURCE_MAIN "${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}.cpp")
  set(OLD_SOURCE_GUI "${CMAKE_SOURCE_DIR}/src/clap-stereo-effect-template-gui.cpp")
  set(NEW_SOURCE_GUI "${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}-gui.cpp")
  
  if(EXISTS "${OLD_SOURCE_MAIN}" AND NOT EXISTS "${NEW_SOURCE_MAIN}")
    message(STATUS "Renaming source file: ${OLD_SOURCE_MAIN} -> ${NEW_SOURCE_MAIN}")
    file(RENAME "${OLD_SOURCE_MAIN}" "${NEW_SOURCE_MAIN}")
  endif()
  
  if(EXISTS "${OLD_SOURCE_GUI}" AND NOT EXISTS "${NEW_SOURCE_GUI}")
    message(STATUS "Renaming GUI source file: ${OLD_SOURCE_GUI} -> ${NEW_SOURCE_GUI}")
    file(RENAME "${OLD_SOURCE_GUI}" "${NEW_SOURCE_GUI}")
  endif()

  # Create embedded font resources
  create_resources(external/mlvg/examples/app/resources build/resources/${TARGET_NAME})

  # Gather plugin source files
  file(GLOB PLUGIN_SOURCES "src/*.cpp" "src/widgets/*.cpp" "src/dsp/*.cpp")
  file(GLOB PLUGIN_HEADERS "src/*.h" "src/widgets/*.h" "src/dsp/*.h")
  
  # Update include statements in source files to use new header names
  foreach(SOURCE_FILE ${PLUGIN_SOURCES})
    if(EXISTS "${SOURCE_FILE}")
      file(READ "${SOURCE_FILE}" SOURCE_CONTENT)
      
      # Replace old header includes with new ones
      string(REPLACE "#include \"clap-stereo-effect-template.h\"" "#include \"${TARGET_NAME}.h\"" SOURCE_CONTENT "${SOURCE_CONTENT}")
      string(REPLACE "#include \"clap-stereo-effect-template-gui.h\"" "#include \"${TARGET_NAME}-gui.h\"" SOURCE_CONTENT "${SOURCE_CONTENT}")
      
      # Write the updated content back
      file(WRITE "${SOURCE_FILE}" "${SOURCE_CONTENT}")
    endif()
  endforeach()

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
    external/clap-host/clap/include
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
    src/widgets
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
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/cmake/${TARGET_NAME}.plist.in
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
  
  # Set platform-specific CLAP install directories (relative to install prefix)
  if(APPLE)
    set(CLAP_INSTALL_DIR "Library/Audio/Plug-Ins/CLAP")
  elseif(UNIX)
    set(CLAP_INSTALL_DIR "lib/clap")
  elseif(WIN32)
    set(CLAP_INSTALL_DIR "CLAP")
  endif()

  # Install to CLAP plugin directory (will be relative to CMAKE_INSTALL_PREFIX)
  install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CLAP_INSTALL_DIR}
    RUNTIME DESTINATION ${CLAP_INSTALL_DIR}
    BUNDLE DESTINATION ${CLAP_INSTALL_DIR}
  )
  
  # Add user install target (no sudo required)
  if(APPLE)
    set(CLAP_USER_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/CLAP")
  elseif(UNIX)
    set(CLAP_USER_INSTALL_DIR "$ENV{HOME}/.clap")
  elseif(WIN32)
    set(CLAP_USER_INSTALL_DIR "$ENV{APPDATA}/CLAP")
  endif()
  
  install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CLAP_USER_INSTALL_DIR}
    RUNTIME DESTINATION ${CLAP_USER_INSTALL_DIR}
    BUNDLE DESTINATION ${CLAP_USER_INSTALL_DIR}
    COMPONENT "CLAP-User"
  )
  
  add_custom_target(install-clap-user
    COMMAND ${CMAKE_COMMAND} --install . --component CLAP-User
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Installing CLAP plugin to user directory..."
    VERBATIM
  )

  # Print build information
  message(STATUS "Plugin: ${PLUGIN_NAME} v${PLUGIN_VERSION}")
  message(STATUS "Creator: ${PLUGIN_CREATOR}")
  message(STATUS "CLAP ID: ${PLUGIN_CLAP_ID}")
  message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/clap")
  message(STATUS "Install directory: ${CLAP_INSTALL_DIR} (relative to CMAKE_INSTALL_PREFIX)")
  message(STATUS "Run `make install` to install to system CLAP directories.")
  message(STATUS "Run `make install-clap-user` to install to user CLAP directories.")
endfunction()
