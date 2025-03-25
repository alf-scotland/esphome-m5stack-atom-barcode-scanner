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

### Method 2: Custom OTA Configuration

You can create your own OTA-enabled firmware by modifying the `firmware.yaml`:

```yaml
# Enable OTA updates with repository support
ota:
  - platform: esphome  # Standard OTA
    password: !secret ota_password
  - platform: repository  # Git repository OTA
    repository:
      type: git
      url: https://github.com/scotland/esphome-m5stack-atom-barcode-scanner
      ref: main  # Change to a specific branch or tag as needed
      path: firmware.yaml
      refresh: 24h  # Check for updates once per day
```

Key configuration options:

- **ref**: The branch or tag to update from (e.g., `main`, `v2024.6.0`, `feature/new-feature`)
- **path**: Path to the YAML file in the repository
- **refresh**: How often to check for updates (e.g., `1h`, `24h`, `7d`)

## Using OTA for Different Purposes

### Stable Channel

For the most stable experience, point to a release tag:

```yaml
ref: v2024.6.0  # Replace with actual version
```

### Beta Testing Channel

For beta testing new features:

```yaml
ref: v2024.6.0-beta.1  # Replace with actual beta version
```

### Development Channel

For cutting-edge features and fixes:

```yaml
ref: main  # Main development branch
```

### Feature Testing

For testing a specific feature:

```yaml
ref: feature/my-new-feature  # Feature branch name
```

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

### Disabling Safe Mode

For testing experimental updates, you may want to disable safe mode:

```yaml
ota:
  - platform: repository
    safe_mode: false  # Disables automatic rollback
    repository:
      # configuration continues...
```

### Custom Update Intervals

You can customize how often the device checks for updates:

```yaml
ota:
  - platform: repository
    repository:
      refresh: 1h  # Check every hour
```

### Conditional Updates

For advanced use cases, you can make updates conditional:

```yaml
ota:
  - platform: repository
    on_update:
      then:
        - if:
            condition:
              # Only update during night hours
              time.is_in_range:
                start: "02:00:00"
                end: "05:00:00"
            then:
              # Proceed with update
              - logger.log: "Updating firmware during maintenance window"
            else:
              # Postpone update
              - logger.log: "Update available but outside maintenance window"
              - delay: 6h
              - repository.check_update:
```

## Best Practices

1. **Test thoroughly**: Always test updates on a test device before deploying to production
2. **Version control**: Keep track of which versions are running on which devices
3. **Update windows**: Schedule updates during low-usage periods
4. **Backup configurations**: Always keep backups of working configurations
5. **Monitor logs**: Check logs after updates to verify successful installation
