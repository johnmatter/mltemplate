#!/usr/bin/env python3
"""
Generate CLAP entry point from metadata.
Usage: python3 scripts/generate_entry_point.py plugin-metadata.json
"""

import json
import sys
import os

def generate_entry_point(metadata):
    """Generate CLAP entry point content from metadata."""
    template = f'''#include "clap-stereo-effect-template.h"
#include "clap-stereo-effect-template-gui.h"
#include <CLAPExport.h>
MADRONALIB_EXPORT_CLAP_PLUGIN_WITH_GUI(ClapStereoEffectTemplate, ClapStereoEffectTemplateGUI, "{metadata["clap_name"]}", "{metadata["clap_vendor"]}")
'''
    return template

def main():
    # check args
    if len(sys.argv) != 2:
        print("Usage: python3 scripts/generate_entry_point.py <json_file>")
        sys.exit(1)

    # check json exists
    json_file = sys.argv[1]
    if not os.path.exists(json_file):
        print(f"Error: File {json_file} not found")
        sys.exit(1)
    
    # generate entry point
    try:
        with open(json_file, 'r') as f:
            metadata = json.load(f)
        
        entry_point_content = generate_entry_point(metadata)
        print(entry_point_content)
    except Exception as e:
        print(f"Error generating entry point: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
