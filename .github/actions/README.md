# GitHub Actions Modules

This directory contains modular GitHub Actions for building and validating the CLAP plugin project.

## Action Overview

### Core Build Pipeline

1. **`parse-workflow-trigger`** - Analyze commit messages and determine build platforms
2. **`setup-dependencies`** - Install platform-specific build dependencies
3. **`build-madronalib`** - Build and install madronalib dependency  
4. **`build-mlvg`** - Build and install mlvg graphics library
5. **`build-plugin`** - Build the CLAP plugin and optional wrappers
6. **`validate-plugin`** - Validate plugin formats using clap-validator
7. **`upload-artifacts`** - Upload artifacts and create releases

### Utility Actions

- **`compress-artifacts`** - Compress plugins to reduce storage usage

## Usage Examples

### Trigger Analysis
```yaml
- uses: ./.github/actions/parse-workflow-trigger
  with:
    event_name: ${{ github.event_name }}
    commit_message: ${{ github.event.head_commit.message }}
```

### Basic Build
```yaml
- uses: ./.github/actions/setup-dependencies
  with:
    platform: linux

- uses: ./.github/actions/build-madronalib  
  with:
    platform: linux

- uses: ./.github/actions/build-mlvg
  with:
    platform: linux

- uses: ./.github/actions/build-plugin
  with:
    platform: linux
    build_wrappers: true
    build_clap_tools: true
```

### Validation and Upload
```yaml
- uses: ./.github/actions/validate-plugin
  with:
    platform: linux
    validate_clap: true
    validate_vst3: false

- uses: ./.github/actions/upload-artifacts
  with:
    platform: linux
    upload_to_artifacts: true
    upload_to_release: true
    compress_artifacts: false
    github_token: ${{ secrets.GITHUB_TOKEN }}
```

## Platform Support

All actions support these platforms:
- `linux` - Ubuntu latest
- `windows` - Windows latest  
- `macos` - macOS latest

## Configuration Options

### Build Options
- **`build_wrappers`** - Build VST3/AU wrappers (default: true)
- **`build_clap_tools`** - Build CLAP development tools (default: true)

### Validation Options  
- **`validate_clap`** - Validate CLAP plugin (default: true)
- **`validate_vst3`** - Validate VST3 plugin (default: false)
- **`validate_au`** - Validate AU plugin, macOS only (default: false)

### Upload Options
- **`upload_to_artifacts`** - Upload to GitHub Actions artifacts (default: true)
- **`upload_to_release`** - Upload to GitHub Releases if tagged (default: true)
- **`compress_artifacts`** - Compress before upload (default: false)
- **`artifact_retention_days`** - Artifact retention period (default: 3)

## Triggering Builds

Use commit message tags to control builds:

```bash
# Build all platforms
git commit -m "feat: new feature [ci-build]"

# Build specific platforms
git commit -m "fix: linux issue [ci-build-linux]"
git commit -m "fix: windows issue [ci-build-windows]"  
git commit -m "fix: macos issue [ci-build-macos]"
```

## Manual Workflow Dispatch

```bash
# Basic build
gh workflow run build-and-validate.yml

# Build with all options
gh workflow run build-and-validate.yml \
  -f build_wrappers=true \
  -f validate_all_formats=true \
  -f compress_artifacts=true
```

## Storage Optimization

To reduce GitHub Actions storage usage:

1. **Enable compression**:
   ```yaml
   compress_artifacts: true
   ```

2. **Reduce retention**:
   ```yaml
   artifact_retention_days: 1  # Minimum: 1 day
   ```

3. **Selective uploads**:
   ```yaml
   upload_to_artifacts: false  # Skip artifacts for releases
   ```

## Platform Support

The workflow runs on GitHub-hosted runners:
- `ubuntu-latest` for Linux builds
- `windows-latest` for Windows builds
- `macos-latest` for macOS builds

## Troubleshooting

### Common Issues

1. **Missing dependencies**: Check `setup-dependencies` action logs
2. **Build failures**: Verify CMake configuration in `build-plugin` action
3. **Validation errors**: Check plugin files exist before validation
4. **Upload failures**: Verify artifact paths and GitHub token permissions

### Debug Steps

1. Enable step debugging:
   ```yaml
   env:
     ACTIONS_STEP_DEBUG: true
   ```

2. List build artifacts:
   ```bash
   find build -name "*.clap" -o -name "*.vst3" -o -name "*.component"
   ```

3. Check workflow summary for upload details
