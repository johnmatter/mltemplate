# CLAPHost.cmake - CMake module for building clap-host
# Usage: include(CLAPHost)

# Function to build clap-host
function(build_clap_host)
  set(HOST_DIR ${CMAKE_SOURCE_DIR}/external/clap-host)
  set(HOST_BUILD_DIR ${CMAKE_BINARY_DIR}/external/clap-host)
  
  # Create build directory
  file(MAKE_DIRECTORY ${HOST_BUILD_DIR})
  
  # Configure clap-host with CMake
  add_custom_target(clap-host-configure ALL
    COMMAND ${CMAKE_COMMAND} -B${HOST_BUILD_DIR} ${HOST_DIR}
    WORKING_DIRECTORY ${HOST_BUILD_DIR}
    COMMENT "Configuring clap-host..."
    VERBATIM
  )
  
  # Build clap-host
  add_custom_target(clap-host-build ALL
    COMMAND ${CMAKE_COMMAND} --build ${HOST_BUILD_DIR}
    DEPENDS clap-host-configure
    COMMENT "Building clap-host..."
    VERBATIM
  )
  
  # Set the host executable path
  set(CLAP_HOST_EXE ${HOST_BUILD_DIR}/host/clap-host
      PARENT_SCOPE)
  
  # Add custom target for running host
  add_custom_target(clap-host-test
    COMMAND ${CLAP_HOST_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
    DEPENDS clap-host-build
    COMMENT "Running clap-host with built plugins..."
    VERBATIM
  )
  
  message(STATUS "clap-host will be built at: ${CLAP_HOST_EXE}")
endfunction()

# Build the host
build_clap_host()
