#!/usr/bin/env python3
"""
Parse GitHub workflow trigger conditions and determine build platforms.

This script analyzes commit messages, event types, and other conditions
to determine if a workflow should run and which platforms to build.
"""

import json
import re
import sys
import os
from typing import List, Dict, Any


class WorkflowTriggerParser:
    """Parse workflow trigger conditions and determine build configuration."""
    
    # Supported platforms
    PLATFORMS = ["linux", "windows", "macos"]
    
    # Commit message patterns
    CI_BUILD_PATTERN = r'\[ci-build(?:-(?:ubuntu|linux|windows|macos))?\]'
    CI_VALIDATE_PATTERN = r'\[ci-validate(?:-(?:clap|vst3|auv2))?\]'
    
    PLATFORM_PATTERNS = {
        "linux": r'\[ci-build-(?:ubuntu|linux)\]',
        "windows": r'\[ci-build-windows\]',
        "macos": r'\[ci-build-macos\]'
    }
    
    VALIDATION_PATTERNS = {
        "clap": r'\[ci-validate-clap\]',
        "vst3": r'\[ci-validate-vst3\]',
        "auv2": r'\[ci-validate-auv2\]'
    }
    
    def __init__(self, event_name: str, commit_message: str = ""):
        """
        Initialize the parser.
        
        Args:
            event_name: GitHub event name (push, pull_request, schedule, etc.)
            commit_message: Commit message to parse for build tags
        """
        self.event_name = event_name
        self.commit_message = commit_message or ""
        
    def should_run_workflow(self) -> bool:
        """
        Determine if the workflow should run.
        
        Returns:
            True if workflow should run, False otherwise
        """
        # Always run for certain event types
        if self.event_name in ["schedule", "workflow_dispatch", "release"]:
            return True
            
        # Check for CI build or validate tags in commit message
        return (bool(re.search(self.CI_BUILD_PATTERN, self.commit_message)) or
                bool(re.search(self.CI_VALIDATE_PATTERN, self.commit_message)))
    
    def get_build_platforms(self) -> List[str]:
        """
        Determine which platforms to build.
        
        Returns:
            List of platform names to build
        """
        if not self.should_run_workflow():
            return []
            
        # Always build all platforms for certain events
        if self.event_name in ["schedule", "workflow_dispatch", "release"]:
            return self.PLATFORMS.copy()
            
        # Check for general [ci-build] tag (build all platforms)
        if re.search(r'\[ci-build\]', self.commit_message):
            return self.PLATFORMS.copy()
            
        # Check for specific platform tags
        selected_platforms = []
        for platform, pattern in self.PLATFORM_PATTERNS.items():
            if re.search(pattern, self.commit_message):
                selected_platforms.append(platform)
                
        return selected_platforms
    
    def get_validation_formats(self) -> List[str]:
        """
        Determine which plugin formats to validate.
        
        Returns:
            List of format names to validate (clap, vst3, auv2)
        """
        if not self.should_run_workflow():
            return []
            
        # Always validate all formats for certain events
        if self.event_name in ["schedule", "workflow_dispatch", "release"]:
            return ["clap", "vst3", "auv2"]
            
        # Check for general [ci-validate] tag (validate all formats)
        if re.search(r'\[ci-validate\]', self.commit_message):
            return ["clap", "vst3", "auv2"]
            
        # Check for specific validation format tags
        selected_formats = []
        for format_name, pattern in self.VALIDATION_PATTERNS.items():
            if re.search(pattern, self.commit_message):
                selected_formats.append(format_name)
                
        return selected_formats
    
    def get_workflow_config(self) -> Dict[str, Any]:
        """
        Get complete workflow configuration.
        
        Returns:
            Dictionary with workflow configuration
        """
        should_run = self.should_run_workflow()
        platforms = self.get_build_platforms()
        validation_formats = self.get_validation_formats()
        
        return {
            "should_run": should_run,
            "platforms": platforms,
            "platforms_json": json.dumps(platforms),
            "validation_formats": validation_formats,
            "validation_formats_json": json.dumps(validation_formats),
            "event_name": self.event_name,
            "commit_message": self.commit_message,
            "trigger_reason": self._get_trigger_reason(should_run, platforms, validation_formats)
        }
    
    def _get_trigger_reason(self, should_run: bool, platforms: List[str], validation_formats: List[str] = None) -> str:
        """Get human-readable reason for trigger decision."""
        if not should_run:
            return "No CI build or validate tags found in commit message"
            
        if self.event_name in ["schedule", "workflow_dispatch", "release"]:
            return f"Triggered by {self.event_name} event"
            
        reasons = []
        
        # Check for build triggers
        if re.search(r'\[ci-build\]', self.commit_message):
            reasons.append("General [ci-build] tag found")
        elif platforms:
            reasons.append(f"Platform-specific build tags: {', '.join(platforms)}")
            
        # Check for validation triggers
        if re.search(r'\[ci-validate\]', self.commit_message):
            reasons.append("General [ci-validate] tag found")
        elif validation_formats:
            reasons.append(f"Format-specific validation tags: {', '.join(validation_formats)}")
            
        return "; ".join(reasons) if reasons else "Unknown trigger reason"


def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Parse GitHub workflow trigger conditions"
    )
    parser.add_argument(
        "--event-name",
        required=True,
        help="GitHub event name (push, pull_request, schedule, etc.)"
    )
    parser.add_argument(
        "--commit-message",
        default="",
        help="Commit message to parse for build tags"
    )
    parser.add_argument(
        "--output-format",
        choices=["json", "github", "shell"],
        default="github",
        help="Output format (default: github)"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    
    args = parser.parse_args()
    
    # Create parser and get configuration
    trigger_parser = WorkflowTriggerParser(args.event_name, args.commit_message)
    config = trigger_parser.get_workflow_config()
    
    if args.verbose:
        print(f"Event: {config['event_name']}", file=sys.stderr)
        print(f"Commit: {config['commit_message'][:50]}...", file=sys.stderr)
        print(f"Reason: {config['trigger_reason']}", file=sys.stderr)
    
    # Output in requested format
    if args.output_format == "json":
        print(json.dumps(config, indent=2))
    elif args.output_format == "github":
        # GitHub Actions output format
        print(f"should-run={str(config['should_run']).lower()}")
        print(f"platforms={config['platforms_json']}")
        print(f"validation-formats={config['validation_formats_json']}")
        print(f"trigger-reason={config['trigger_reason']}")
    elif args.output_format == "shell":
        # Shell variable format
        print(f"SHOULD_RUN={str(config['should_run']).lower()}")
        print(f"PLATFORMS='{config['platforms_json']}'")
        print(f"VALIDATION_FORMATS='{config['validation_formats_json']}'")
        print(f"TRIGGER_REASON='{config['trigger_reason']}'")
    
    return 0 if config['should_run'] else 1


if __name__ == "__main__":
    sys.exit(main())
