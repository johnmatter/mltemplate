# CLAPWrapper.cmake - CMake module for creating VST3 and AU2 wrappers using clap-wrapper
# Usage: include(CLAPWrapper)
# Note: Requires PLUGIN_PROJECT_NAME and other metadata variables to be set

# Function to create VST3 and AU2 wrappers for a CLAP plugin
function(create_clap_wrappers TARGET_NAME)
  # clap-wrapper subdirectory should already be added in main CMakeLists.txt
  # VST3 test host can be built separately using: cmake --build build --target vst3-testhost
  # This builds audiohost and editorhost from the VST3 SDK with hosting examples enabled

  
  # Create VST3 wrapper
  set(VST3_TARGET ${TARGET_NAME}_vst3)
  add_library(${VST3_TARGET} MODULE)
  target_add_vst3_wrapper(
    TARGET ${VST3_TARGET}
    OUTPUT_NAME "${PLUGIN_BUNDLE_NAME}"
    SUPPORTS_ALL_NOTE_EXPRESSIONS TRUE
  )
  
  # Set VST3-specific properties
  set_target_properties(${VST3_TARGET} PROPERTIES
    CLAP_VST3_TUID_STRING "${PLUGIN_VST3_TUID}"
    CLAP_VST3_SUBCATEGORIES "Fx|Stereo"
    CLAP_VST3_VENDOR "${PLUGIN_VENDOR}"
    CLAP_VST3_VERSION "${PLUGIN_VERSION}"
  )
  
  # Create AU2 wrapper (macOS only)
  if(APPLE)
    set(AU2_TARGET ${TARGET_NAME}_au2)
    add_library(${AU2_TARGET} MODULE)
    
    # Set AU2-specific variables globally before calling target_add_auv2_wrapper
    set(AUV2_SUBTYPE_CODE "${PLUGIN_AU2_SUBTYPE}" CACHE STRING "AU2 subtype code")
    set(AUV2_MANUFACTURER_CODE "${PLUGIN_AU2_MANUFACTURER}" CACHE STRING "AU2 manufacturer code")
    set(AUV2_MANUFACTURER_NAME "${PLUGIN_VENDOR}" CACHE STRING "AU2 manufacturer name")
    set(AUV2_INSTRUMENT_TYPE "aufx" CACHE STRING "AU2 instrument type")
    set(AUV2_BUNDLE_VERSION "${PLUGIN_VERSION}" CACHE STRING "AU2 bundle version")
    
    target_add_auv2_wrapper(
      TARGET ${AU2_TARGET}
      OUTPUT_NAME "${PLUGIN_BUNDLE_NAME}"
      SUPPORTS_ALL_NOTE_EXPRESSIONS TRUE
    )
  endif()
  

  
  # Platform-specific configuration for wrappers
  if(APPLE)
    # macOS-specific settings for all wrappers
    foreach(WRAPPER_TARGET ${VST3_TARGET} ${AU2_TARGET})
      set_target_properties(${WRAPPER_TARGET} PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "${PLUGIN_BUNDLE_IDENTIFIER}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PLUGIN_BUNDLE_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PLUGIN_BUNDLE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PLUGIN_BUNDLE_SHORT_VERSION}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PLUGIN_BUNDLE_LONG_VERSION}"
        MACOSX_BUNDLE_INFO_STRING "${PLUGIN_BUNDLE_INFO_STRING}"
        MACOSX_BUNDLE_COPYRIGHT "${PLUGIN_BUNDLE_COPYRIGHT}"
      )
    endforeach()
  elseif(WIN32)
    # Windows-specific settings
    set_target_properties(${VST3_TARGET} PROPERTIES
      WIN32_EXECUTABLE TRUE
    )
  endif()
  
  # Set output directories
  set_target_properties(${VST3_TARGET} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/vst3"
  )
  
  if(APPLE)
    set_target_properties(${AU2_TARGET} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/au2"
    )
  endif()
  
  # Add custom install targets
  install(TARGETS ${VST3_TARGET}
    LIBRARY DESTINATION "VST3"
    COMPONENT "VST3"
  )
  
  if(APPLE)
    install(TARGETS ${AU2_TARGET}
      LIBRARY DESTINATION "AudioUnit"
      COMPONENT "AudioUnit"
    )
  endif()
  

  
  # Print status messages
  message(STATUS "Created VST3 wrapper: ${VST3_TARGET}")
  if(APPLE)
    message(STATUS "Created AU2 wrapper: ${AU2_TARGET}")
  endif()
endfunction()

# Function to set up wrapper-specific metadata
function(setup_wrapper_metadata)
  # Set default values for wrapper metadata if not already set
  if(NOT DEFINED PLUGIN_VST3_TUID)
    # Generate a TUID based on the plugin name (this is a simplified approach)
    string(TOUPPER "${PLUGIN_PROJECT_NAME}" UPPER_NAME)
    set(PLUGIN_VST3_TUID "1234567890abcdef" CACHE STRING "VST3 TUID for the plugin")
  endif()
  
  if(NOT DEFINED PLUGIN_AU2_SUBTYPE)
    # Generate a 4-character subtype from the plugin name
    string(SUBSTRING "${PLUGIN_PROJECT_NAME}" 0 4 AU2_SUBTYPE)
    string(TOUPPER "${AU2_SUBTYPE}" AU2_SUBTYPE)
    set(PLUGIN_AU2_SUBTYPE "${AU2_SUBTYPE}" CACHE STRING "AU2 subtype for the plugin")
  endif()
  
  if(NOT DEFINED PLUGIN_AU2_MANUFACTURER)
    # Use vendor name or default
    if(DEFINED PLUGIN_VENDOR)
      string(SUBSTRING "${PLUGIN_VENDOR}" 0 4 AU2_MANUFACTURER)
      string(TOUPPER "${AU2_MANUFACTURER}" AU2_MANUFACTURER)
    else()
      set(AU2_MANUFACTURER "MADR")
    endif()
    set(PLUGIN_AU2_MANUFACTURER "${AU2_MANUFACTURER}" CACHE STRING "AU2 manufacturer code for the plugin")
  endif()
endfunction()
