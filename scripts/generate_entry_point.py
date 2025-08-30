#!/usr/bin/env python3
"""
Generate CLAP entry point from metadata.
Usage: python3 scripts/generate_entry_point.py plugin-metadata.json
"""

import json
import sys
import os

def get_clap_features(metadata):
    """Get CLAP features based on plugin type."""
    plugin_type = metadata.get("plugin_type", "instrument")
    plugin_category = metadata.get("plugin_category", "synthesizer")
    
    features = []
    
    # Main plugin type
    if plugin_type == "audio-effect":
        features.append("CLAP_PLUGIN_FEATURE_AUDIO_EFFECT")
    elif plugin_type == "instrument":
        features.append("CLAP_PLUGIN_FEATURE_INSTRUMENT")
    elif plugin_type == "note-effect":
        features.append("CLAP_PLUGIN_FEATURE_NOTE_EFFECT")
    elif plugin_type == "note-detector":
        features.append("CLAP_PLUGIN_FEATURE_NOTE_DETECTOR")
    elif plugin_type == "analyzer":
        features.append("CLAP_PLUGIN_FEATURE_ANALYZER")
    else:
        features.append("CLAP_PLUGIN_FEATURE_INSTRUMENT")  # default
    
    # Plugin category
    if plugin_category == "distortion":
        features.append("CLAP_PLUGIN_FEATURE_DISTORTION")
    elif plugin_category == "synthesizer":
        features.append("CLAP_PLUGIN_FEATURE_SYNTHESIZER")
    elif plugin_category == "filter":
        features.append("CLAP_PLUGIN_FEATURE_FILTER")
    elif plugin_category == "reverb":
        features.append("CLAP_PLUGIN_FEATURE_REVERB")
    elif plugin_category == "delay":
        features.append("CLAP_PLUGIN_FEATURE_DELAY")
    elif plugin_category == "compressor":
        features.append("CLAP_PLUGIN_FEATURE_COMPRESSOR")
    elif plugin_category == "equalizer":
        features.append("CLAP_PLUGIN_FEATURE_EQUALIZER")
    elif plugin_category == "utility":
        features.append("CLAP_PLUGIN_FEATURE_UTILITY")
    else:
        features.append("CLAP_PLUGIN_FEATURE_SYNTHESIZER")  # default
    
    # Always add stereo for this template
    features.append("CLAP_PLUGIN_FEATURE_STEREO")
    
    return features

def generate_entry_point(metadata):
    """Generate CLAP entry point content from metadata."""
    features = get_clap_features(metadata)
    features_str = ",\n    ".join(features)
    
    plugin_type = metadata.get("plugin_type", "instrument")
    plugin_category = metadata.get("plugin_category", "synthesizer")
    
    # Determine description based on plugin type
    if plugin_type == "audio-effect":
        description = f"{plugin_category.title()} Effect"
    elif plugin_type == "instrument":
        description = f"{plugin_category.title()}"
    else:
        description = f"{plugin_type.title()}"
    
    template = f'''#include "clap-stereo-effect-template.h"
#include "clap-stereo-effect-template-gui.h"
#include <CLAPExport.h>

extern "C" {{
  static const char* const features[] = {{
    {features_str},
    nullptr
  }};
  
  static const clap_plugin_descriptor desc = {{
    CLAP_VERSION_INIT,
    "{metadata["clap_name"]}-id",
    "{metadata["clap_name"]}",
    "{metadata["clap_vendor"]}",
    "https://madronalabs.com",
    "",
    "",
    "{metadata["version"]}",
    "{description}",
    features
  }};
  
  static const clap_plugin* plugin_create(const clap_plugin_factory* factory, const clap_host* host, const char* plugin_id) {{
    if (!host) {{
      return nullptr;
    }}
    if (!clap_version_is_compatible(host->clap_version)) {{
      return nullptr;
    }}
    if (!plugin_id) {{
      return nullptr;
    }}
    if (strcmp(plugin_id, desc.id) != 0) {{
      return nullptr;
    }}
    return new ml::CLAPPluginWrapper<ClapStereoEffectTemplate, ClapStereoEffectTemplateGUI>(host, &desc);
  }}
  
  static const clap_plugin_factory plugin_factory = {{
    [](const clap_plugin_factory* factory) -> uint32_t {{
      return 1;
    }},
    [](const clap_plugin_factory* factory, uint32_t index) -> const clap_plugin_descriptor* {{
      return index == 0 ? &desc : nullptr;
    }},
    plugin_create
  }};
  
  const CLAP_EXPORT clap_plugin_entry clap_entry = {{
    CLAP_VERSION_INIT,
    [](const char* path) -> bool {{ return true; }},
    []() {{}},
    [](const char* factory_id) -> const void* {{
      return strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0 ? &plugin_factory : nullptr;
    }}
  }};
}}
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
