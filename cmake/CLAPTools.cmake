# CLAPTools.cmake - Master module for all CLAP development tools
# Usage: include(CLAPTools)
# 
# This module provides:
# - clap-validator: Plugin validation and testing
# - clap-info: Plugin information and inspection
# - clap-host: Standalone plugin testing host
#
# Available targets:
# - clap-tools-build: Build all tools
# - clap-validator-validate: Run validation on built plugins
# - clap-info-scan: Scan plugin information
# - clap-host-test: Test plugins in standalone host
# - clap-test-suite: Run all tests and validations

# Include individual tool modules
include(CLAPValidator)
include(CLAPInfo)
include(CLAPHost)

# Create a master target that builds all tools
add_custom_target(clap-tools-build ALL
  DEPENDS clap-validator-build clap-info-build clap-host-build
  COMMENT "Building all CLAP development tools..."
  VERBATIM
)

# Create a comprehensive test suite target
add_custom_target(clap-test-suite
  COMMAND ${CMAKE_COMMAND} -E echo "=== Running CLAP Test Suite ==="
  COMMAND ${CMAKE_COMMAND} -E echo "1. Plugin Information:"
  COMMAND ${CLAP_INFO_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "2. Plugin Validation:"
  COMMAND ${CLAP_VALIDATOR_EXE} validate ${CMAKE_BINARY_DIR}/clap/*.clap
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "3. Standalone Host Test:"
  COMMAND ${CLAP_HOST_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
  DEPENDS clap-tools-build
  COMMENT "Running complete CLAP test suite..."
  VERBATIM
)

# Add convenience targets for individual operations
add_custom_target(clap-validate
  COMMAND ${CLAP_VALIDATOR_EXE} validate ${CMAKE_BINARY_DIR}/clap/*.clap
  DEPENDS clap-validator-build
  COMMENT "Validating CLAP plugins..."
  VERBATIM
)

add_custom_target(clap-info
  COMMAND ${CLAP_INFO_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
  DEPENDS clap-info-build
  COMMENT "Getting CLAP plugin information..."
  VERBATIM
)

add_custom_target(clap-host
  COMMAND ${CLAP_HOST_EXE} ${CMAKE_BINARY_DIR}/clap/*.clap
  DEPENDS clap-host-build
  COMMENT "Testing CLAP plugins in standalone host..."
  VERBATIM
)

# Print status information
message(STATUS "CLAP Tools Configuration:")
message(STATUS "  clap-validator: ${CLAP_VALIDATOR_EXE}")
message(STATUS "  clap-info: ${CLAP_INFO_EXE}")
message(STATUS "  clap-host: ${CLAP_HOST_EXE}")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  clap-tools-build    - Build all tools")
message(STATUS "  clap-test-suite     - Run complete test suite")
message(STATUS "  clap-validate       - Validate plugins")
message(STATUS "  clap-info           - Get plugin information")
message(STATUS "  clap-host           - Test in standalone host")
