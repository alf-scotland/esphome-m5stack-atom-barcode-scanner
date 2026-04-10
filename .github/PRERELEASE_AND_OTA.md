# Pre-releases and OTA Updates

This document explains how to work with pre-release builds (beta/rc) and use OTA (Over-The-Air) updates with your ESPHome M5Stack Atom Barcode Scanner.

## Pre-release Workflow

Pre-releases allow you to test new features or fixes before they're officially released.

### Creating a Pre-release

1. Update `project_version` in the `substitutions:` block of `firmware/atom_lite.yaml`:
   ```yaml
   substitutions:
     project_version: "YYYY.MM.0-beta.1"
   ```

2. Commit the version bump to `main`.

3. Tag the pre-release and push:
   ```bash
   git tag vYYYY.MM.0-beta.1
   git push origin vYYYY.MM.0-beta.1
   ```

4. The CI system will automatically:
   - Build the firmware
   - Create a GitHub pre-release
   - Mark it as a pre-release on GitHub
   - Attach `firmware.bin`, `firmware.factory.bin`, and `firmware.ota.bin` as release assets

### Pre-release Naming Conventions

Follow these naming patterns:

- Beta releases: `vYYYY.MM.0-beta.N` (e.g., `v2026.4.0-beta.1`)
- Release candidates: `vYYYY.MM.0-rc.N` (e.g., `v2026.4.0-rc.1`)

## OTA Updates

OTA (Over-The-Air) updates allow you to update your device without physically connecting to it.

### Using OTA with Pre-releases

#### Method 1: Manual OTA Upload via ESPHome Dashboard

1. Download `firmware.bin` from the GitHub pre-release.
2. In the ESPHome dashboard:
   - Navigate to your device.
   - Click **Upload Firmware**.
   - Select the downloaded binary and click **Upload**.

#### Method 2: Integration with Home Assistant

1. In Home Assistant, go to **Settings** → **Devices & Services** → **ESPHome**.
2. Find your device and click on it.
3. Click the three dots (⋮) and select **OTA Update**.
4. Choose the `firmware.bin` binary you downloaded from the pre-release.
5. Click **Upload**.

### OTA Configuration (Advanced)

You can flash a specific binary via the `http_request` OTA platform. Use the modern list
format (ESPHome 2024.6+):

```yaml
# Standard OTA via ESPHome dashboard
ota:
  - platform: esphome

# Optional: enable safe mode recovery (recommended)
safe_mode:
  reboot_timeout: 3min
  num_attempts: 5

# HTTP OTA from a direct URL (e.g., GitHub Release binary)
ota:
  - platform: http_request

http_request:
  verify_ssl: false

button:
  - platform: template
    name: "Flash Pre-release"
    on_press:
      - ota.http_request.flash:
          url: https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner/releases/download/vYYYY.MM.0-beta.1/firmware.ota.bin
          verify_ssl: false
```

## Testing Pre-releases

When testing pre-releases:

1. Always backup your working configuration.
2. Document any issues you find in GitHub issues.
3. Include logs and steps to reproduce.
4. If possible, test on a dedicated device (not your primary device).

## Reverting to Stable Releases

If a pre-release causes issues:

1. Flash the latest stable release using any of the OTA methods above.
2. Or, compile locally from `main`:
   ```bash
   uv run esphome compile firmware/atom_lite.yaml
   ```

## Contributing Test Results

After testing a pre-release, please provide feedback:

1. For bugs, create an issue with the prefix `[Beta]` or `[RC]`.
2. For successful tests, comment on the GitHub release.
3. Include details about your testing environment.
