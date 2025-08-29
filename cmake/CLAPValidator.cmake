# CLAPValidator.cmake - CMake module for building clap-validator
# Usage: include(CLAPValidator)

# Check if Rust is available
find_program(CARGO cargo)
if(NOT CARGO)
  message(WARNING "Cargo not found. clap-validator will not be built.")
  return()
endif()

# Function to build clap-validator
function(build_clap_validator)
  set(VALIDATOR_DIR ${CMAKE_SOURCE_DIR}/external/clap-validator)
  set(VALIDATOR_BUILD_DIR ${CMAKE_BINARY_DIR}/external/clap-validator)
  
  # Create build directory
  file(MAKE_DIRECTORY ${VALIDATOR_BUILD_DIR})
  
  # Build clap-validator using cargo
  add_custom_target(clap-validator-build ALL
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${VALIDATOR_BUILD_DIR}
    ${CARGO} build --release --manifest-path ${VALIDATOR_DIR}/Cargo.toml
    WORKING_DIRECTORY ${VALIDATOR_DIR}
    COMMENT "Building clap-validator..."
    VERBATIM
  )
  
  # Set the validator executable path
  set(CLAP_VALIDATOR_EXE ${VALIDATOR_BUILD_DIR}/release/clap-validator
      PARENT_SCOPE)
  
  # Add custom target for running validation
  add_custom_target(clap-validator-validate
    COMMAND ${CLAP_VALIDATOR_EXE} validate ${CMAKE_BINARY_DIR}/clap/*.clap
    DEPENDS clap-validator-build
    COMMENT "Running clap-validator on built plugins..."
    VERBATIM
  )
  
  message(STATUS "clap-validator will be built at: ${CLAP_VALIDATOR_EXE}")
endfunction()

# Build the validator
build_clap_validator()
