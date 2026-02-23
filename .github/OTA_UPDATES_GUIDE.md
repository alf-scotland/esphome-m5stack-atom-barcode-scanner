# OTA Updates Guide

This guide explains how to use Over-The-Air (OTA) updates for the ESPHome M5Stack Atom Barcode Scanner project.

## Available OTA Update Methods

The firmware supports three OTA update methods:

1. **ESPHome Dashboard**: Updates via the ESPHome dashboard's built-in OTA mechanism.
2. **HTTP OTA**: Updates via a direct HTTP URL to a firmware binary.
3. **Git Repository OTA**: Automatic updates from a GitHub repository branch/tag.

## Git Repository OTA Updates

The most powerful OTA method is the Git Repository approach, which allows your device to automatically update from a specified GitHub branch or tag.

### How It Works

1. The device periodically checks the specified GitHub repository reference (branch/tag)
2. If the firmware version on GitHub is newer, the device downloads and installs it
3. The device reboots with the new firmware

### Benefits

- Seamless updates without manual intervention
- Easy testing of features from different branches or PRs
- Simple rollbacks by switching to an older tag

## Setting Up OTA Updates

### Method 1: Use Pre-built OTA-enabled Firmware

The easiest way to get started with OTA updates is to use our pre-built OTA-enabled firmware:

1. For stable releases:
   - Download the firmware binary from the [latest release](https://github.com/scotland/esphome-m5stack-atom-barcode-scanner/releases/latest)

2. For pre-releases (beta/rc):
   - Download the `firmware-ota.bin` from the [pre-releases](https://github.com/scotland/esphome-m5stack-atom-barcode-scanner/releases)
   - This firmware is pre-configured to update from that specific pre-release

3. For PR testing:
   - Go to the PR's workflow run
   - Download the firmware artifact
   - This firmware is pre-configured to update from the PR branch

### Method 2: OTA from a Direct URL

Use the `http_request` OTA platform to flash a specific binary hosted on GitHub Releases:

```yaml
ota:
  - platform: esphome
    password: !secret ota_password
  - platform: http_request

http_request:
  verify_ssl: false

button:
  - platform: template
    name: "Update Firmware"
    on_press:
      - ota.http_request.flash:
          url: https://github.com/scotland/esphome-m5stack-atom-barcode-scanner/releases/latest/download/firmware.bin
          verify_ssl: false
```

Change the URL to point to a specific release tag binary when you need a particular version.

## Using OTA for Different Purposes

Download the matching firmware binary from the
[GitHub Releases page](https://github.com/scotland/esphome-m5stack-atom-barcode-scanner/releases)
and flash it via the ESPHome dashboard or the `http_request` OTA button above.

### Stable Channel

Download `firmware.bin` from the latest stable release tag (e.g., `v2024.6.0`).

### Beta Testing Channel

Download `firmware.bin` from the desired pre-release tag (e.g., `v2024.6.0-beta.1`).

### Development Channel

Compile locally from the `main` branch using the ESPHome CLI or dashboard.

## Common OTA Issues and Solutions

### Failed Updates

If an update fails:

1. Check your device's logs for error messages
2. Ensure your device has a stable internet connection
3. Verify the repository reference (branch/tag) exists
4. Try updating with the ESPHome dashboard as a fallback

### Automatic Rollbacks

If a bad update is deployed:

1. The device may roll back automatically if `safe_mode` is enabled
2. You can manually flash an older version
3. Point your OTA config to a known good tag

### Monitoring OTA Status

The OTA update process will be logged in the device logs. You can monitor:

1. Update checks (device looking for new versions)
2. Download progress
3. Installation status
4. Post-update boot messages

## Advanced OTA Configuration

### Safe Mode

ESPHome's `safe_mode` component automatically boots into a recovery mode after repeated
crash loops, allowing you to re-flash the device. Add it to `firmware.yaml`:

```yaml
safe_mode:
  reboot_timeout: 3min
  num_attempts: 5
```

### OTA with Password Protection

Always protect OTA with a password in production to prevent unauthorized updates:

```yaml
ota:
  - platform: esphome
    password: !secret ota_password
```

Store the password in `secrets.yaml`:

```yaml
ota_password: "your-strong-password-here"
```

## Best Practices

1. **Test thoroughly**: Always test updates on a test device before deploying to production
2. **Version control**: Keep track of which versions are running on which devices
3. **Update windows**: Schedule updates during low-usage periods
4. **Backup configurations**: Always keep backups of working configurations
5. **Monitor logs**: Check logs after updates to verify successful installation
