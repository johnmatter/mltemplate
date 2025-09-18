#!/usr/bin/env python3
"""
Test script for the workflow trigger parser.
"""

import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

from parse_workflow_trigger import WorkflowTriggerParser


def test_parser():
    """Test various scenarios for the workflow trigger parser."""
    
    test_cases = [
        # (event_name, commit_message, expected_should_run, expected_platforms, expected_validation_formats)
        ("push", "feat: add new feature", False, [], []),
        ("push", "feat: add new feature [ci-build]", True, ["linux", "windows", "macos"], []),
        ("push", "fix: linux issue [ci-build-linux]", True, ["linux"], []),
        ("push", "fix: windows issue [ci-build-windows]", True, ["windows"], []),
        ("push", "fix: macos issue [ci-build-macos]", True, ["macos"], []),
        ("push", "fix: ubuntu issue [ci-build-ubuntu]", True, ["linux"], []),
        ("push", "fix: multiple [ci-build-linux] [ci-build-windows]", True, ["linux", "windows"], []),
        ("push", "test: validate all [ci-validate]", True, [], ["clap", "vst3", "auv2"]),
        ("push", "test: validate clap [ci-validate-clap]", True, [], ["clap"]),
        ("push", "test: validate vst3 [ci-validate-vst3]", True, [], ["vst3"]),
        ("push", "test: validate auv2 [ci-validate-auv2]", True, [], ["auv2"]),
        ("push", "build and validate [ci-build] [ci-validate-vst3]", True, ["linux", "windows", "macos"], ["vst3"]),
        ("schedule", "", True, ["linux", "windows", "macos"], ["clap", "vst3", "auv2"]),
        ("workflow_dispatch", "", True, ["linux", "windows", "macos"], ["clap", "vst3", "auv2"]),
        ("release", "", True, ["linux", "windows", "macos"], ["clap", "vst3", "auv2"]),
        ("pull_request", "feat: no ci tag", False, [], []),
    ]
    
    print("Testing workflow trigger parser...")
    print("=" * 60)
    
    passed = 0
    failed = 0
    
    for event_name, commit_message, expected_should_run, expected_platforms, expected_validation_formats in test_cases:
        parser = WorkflowTriggerParser(event_name, commit_message)
        config = parser.get_workflow_config()
        
        should_run = config["should_run"]
        platforms = config["platforms"]
        validation_formats = config["validation_formats"]
        
        # Check results
        should_run_ok = should_run == expected_should_run
        platforms_ok = set(platforms) == set(expected_platforms)
        validation_ok = set(validation_formats) == set(expected_validation_formats)
        
        status = "PASS" if (should_run_ok and platforms_ok and validation_ok) else "FAIL"
        
        print(f"{status} | {event_name:15} | {commit_message[:30]:30} | {should_run} | {platforms} | {validation_formats}")
        
        if should_run_ok and platforms_ok and validation_ok:
            passed += 1
        else:
            failed += 1
            if not should_run_ok:
                print(f"    Expected should_run: {expected_should_run}, got: {should_run}")
            if not platforms_ok:
                print(f"    Expected platforms: {expected_platforms}, got: {platforms}")
            if not validation_ok:
                print(f"    Expected validation_formats: {expected_validation_formats}, got: {validation_formats}")
    
    print("=" * 60)
    print(f"Results: {passed} passed, {failed} failed")
    
    return failed == 0


if __name__ == "__main__":
    success = test_parser()
    sys.exit(0 if success else 1)
