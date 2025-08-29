# FontEmbedding.cmake - Clean font embedding for CLAP plugins
# Usage: include(FontEmbedding)
#
# This module provides a simple way to embed custom fonts in CLAP plugins.
# It extends MLVG's create_resources function with font-specific functionality.

# Function to add custom fonts to a plugin
function(add_custom_fonts TARGET_NAME FONT_DIR)
  # Create font resources directory
  set(FONT_RESOURCES_DIR ${CMAKE_BINARY_DIR}/font_resources/${TARGET_NAME})
  file(MAKE_DIRECTORY ${FONT_RESOURCES_DIR})
  
  # Find all font files recursively in subdirectories
  file(GLOB_RECURSE FONT_FILES 
    "${FONT_DIR}/*.otf" 
    "${FONT_DIR}/*.ttf" 
    "${FONT_DIR}/*.OTF" 
    "${FONT_DIR}/*.TTF"
  )
  
  # Create empty main include file
  set(INCLUDE_FILE "${FONT_RESOURCES_DIR}/resources.c")
  file(WRITE "${INCLUDE_FILE}" "")
  
  # Process each font file individually
  foreach(FONT_FILE ${FONT_FILES})
    # Get filename without path
    get_filename_component(FONT_FILENAME ${FONT_FILE} NAME)
    
    # Replace filename spaces & extension separator for C compatibility
    string(REGEX REPLACE "\\.| |-" "_" C_FILENAME ${FONT_FILENAME})
    
    # Read hex data from file
    file(READ ${FONT_FILE} FILEDATA HEX)
    
    # Convert hex data for C compatibility
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," FILEDATA ${FILEDATA})
    
    # Create output file for this font
    set(OUTPUT_FILE "${FONT_RESOURCES_DIR}/${C_FILENAME}.c")
    file(WRITE "${OUTPUT_FILE}" "namespace resources \n{\n")
    file(APPEND "${OUTPUT_FILE}" "const unsigned char ${C_FILENAME}[] = {${FILEDATA}};\n")
    file(APPEND "${OUTPUT_FILE}" "const unsigned ${C_FILENAME}_size = sizeof(${C_FILENAME});\n")
    file(APPEND "${OUTPUT_FILE}" "\n}")
    
    # Append filename to main include file
    file(APPEND "${INCLUDE_FILE}" "#include \"${C_FILENAME}.c\"\n")
    
    message(STATUS "Embedded font: ${FONT_FILENAME} -> ${C_FILENAME}")
  endforeach()
  
  # Add the generated resources to the target
  target_sources(${TARGET_NAME} PRIVATE
    ${FONT_RESOURCES_DIR}/resources.c
  )
  
  # Set the generated resource files to compile as C++
  set_source_files_properties(${FONT_RESOURCES_DIR}/resources.c PROPERTIES COMPILE_FLAGS "-x c++")
  
  # Add include directory for the generated font resources
  target_include_directories(${TARGET_NAME} PRIVATE ${FONT_RESOURCES_DIR})
  
  message(STATUS "Custom fonts embedded for ${TARGET_NAME} from ${FONT_DIR}")
  message(STATUS "Font resources generated in: ${FONT_RESOURCES_DIR}")
endfunction()

# Function to create a fonts directory with example fonts
function(create_fonts_directory)
  set(FONTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fonts)
  file(MAKE_DIRECTORY ${FONTS_DIR})
  
  # Create a README file explaining how to use custom fonts
  file(WRITE ${FONTS_DIR}/README.md
"# Custom Fonts Directory

Place your custom font files (OTF/TTF) in this directory to embed them in your CLAP plugin.

## Supported Formats
- OpenType Font (.otf)
- TrueType Font (.ttf)

## Usage
1. Add your font files to this directory
2. In your CMakeLists.txt, uncomment and modify:
   \`\`\`cmake
   add_custom_fonts(\${PLUGIN_NAME} \"fonts/\")
   \`\`\`
3. In your GUI class, include the generated resources:
   \`\`\`cpp
   #include \"../build/resources/your-plugin-name/resources.c\"
   #include \"font_loader.h\"
   \`\`\`
4. Load your fonts in initializeResources():
   \`\`\`cpp
   LOAD_EMBEDDED_FONT(_resources, nvg, \"my_font\", my_font_otf);
   \`\`\`

## Font Naming
- Filename: \`my_custom_font.otf\`
- Generated resource: \`resources::my_custom_font_otf\`
- Generated size: \`resources::my_custom_font_otf_size\`
- Usage: \`LOAD_EMBEDDED_FONT(_resources, nvg, \"my_font\", my_custom_font_otf);\`

## Example
If you have \`fonts/Roboto-Bold.ttf\`:
\`\`\`cpp
LOAD_EMBEDDED_FONT(_resources, nvg, \"roboto_bold\", Roboto_Bold_ttf);
\`\`\`
")
  
  message(STATUS "Fonts directory created: ${FONTS_DIR}")
  message(STATUS "Add your custom font files to ${FONTS_DIR}/")
endfunction()

# Function to list available fonts in a directory
function(list_available_fonts FONT_DIR)
  file(GLOB FONT_FILES "${FONT_DIR}/*.otf" "${FONT_DIR}/*.ttf" "${FONT_DIR}/*.OTF" "${FONT_DIR}/*.TTF")
  
  if(FONT_FILES)
    message(STATUS "Available fonts in ${FONT_DIR}:")
    foreach(FONT_FILE ${FONT_FILES})
      get_filename_component(FONT_NAME ${FONT_FILE} NAME_WE)
      message(STATUS "  - ${FONT_NAME}")
    endforeach()
  else()
    message(WARNING "No font files found in ${FONT_DIR}")
  endif()
endfunction()
