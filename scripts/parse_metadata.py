#!/usr/bin/env python3
"""
Parse plugin metadata from JSON and output CMake variables.
Usage: python3 scripts/parse_metadata.py plugin-metadata.json
"""

import json
import sys
import os

def parse_metadata(json_file):
    """Parse JSON metadata and return CMake variable assignments."""
    with open(json_file, 'r') as f:
        metadata = json.load(f)
    
    cmake_vars = []
    
    # Project info
    cmake_vars.append(f'set(PLUGIN_PROJECT_NAME "{metadata["project_name"]}")')
    
    # Basic plugin info
    cmake_vars.append(f'set(PLUGIN_NAME "{metadata["name"]}")')
    cmake_vars.append(f'set(PLUGIN_CREATOR "{metadata["creator"]}")')
    cmake_vars.append(f'set(PLUGIN_VERSION "{metadata["version"]}")')
    
    # Bundle properties
    cmake_vars.append(f'set(PLUGIN_BUNDLE_IDENTIFIER "{metadata["bundle_identifier"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_NAME "{metadata["bundle_name"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_VERSION "{metadata["bundle_version"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_SHORT_VERSION "{metadata["bundle_short_version"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_LONG_VERSION "{metadata["bundle_long_version"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_INFO_STRING "{metadata["bundle_info_string"]}")')
    cmake_vars.append(f'set(PLUGIN_BUNDLE_COPYRIGHT "{metadata["bundle_copyright"]}")')
    
    # CLAP properties
    cmake_vars.append(f'set(PLUGIN_CLAP_ID "{metadata["clap_id"]}")')
    cmake_vars.append(f'set(PLUGIN_CLAP_NAME "{metadata["clap_name"]}")')
    cmake_vars.append(f'set(PLUGIN_CLAP_VENDOR "{metadata["clap_vendor"]}")')
    
    return '\n'.join(cmake_vars)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 scripts/parse_metadata.py <json_file>")
        sys.exit(1)
    
    json_file = sys.argv[1]
    if not os.path.exists(json_file):
        print(f"Error: File {json_file} not found")
        sys.exit(1)
    
    try:
        cmake_output = parse_metadata(json_file)
        print(cmake_output)
    except Exception as e:
        print(f"Error parsing metadata: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
