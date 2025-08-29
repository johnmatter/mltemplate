# CLAPInfo.cmake - CMake module for building clap-info
# Usage: include(CLAPInfo)

# Function to build clap-info
function(build_clap_info)
  set(INFO_DIR ${CMAKE_SOURCE_DIR}/external/clap-info)
  set(INFO_BUILD_DIR ${CMAKE_BINARY_DIR}/external/clap-info)
  
  # Create build directory
  file(MAKE_DIRECTORY ${INFO_BUILD_DIR})
  
  # Configure clap-info with CMake
  add_custom_target(clap-info-configure ALL
    COMMAND ${CMAKE_COMMAND} -B${INFO_BUILD_DIR} ${INFO_DIR}
    WORKING_DIRECTORY ${INFO_BUILD_DIR}
    COMMENT "Configuring clap-info..."
    VERBATIM
  )
  
  # Build clap-info
  add_custom_target(clap-info-build ALL
    COMMAND ${CMAKE_COMMAND} --build ${INFO_BUILD_DIR}
    DEPENDS clap-info-configure
    COMMENT "Building clap-info..."
    VERBATIM
  )
  
  # Set the info executable path
  set(CLAP_INFO_EXE ${INFO_BUILD_DIR}/clap-info
      PARENT_SCOPE)
  
  # Add custom target for running info
  add_custom_target(clap-info-scan
    COMMAND ${CLAP_INFO_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
    DEPENDS clap-info-build
    COMMENT "Running clap-info on built plugins..."
    VERBATIM
  )
  
  message(STATUS "clap-info will be built at: ${CLAP_INFO_EXE}")
endfunction()

# Build the info tool
build_clap_info()
