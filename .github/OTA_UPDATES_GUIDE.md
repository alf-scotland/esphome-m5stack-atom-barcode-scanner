# OTA Updates Guide

This guide explains how to use Over-The-Air (OTA) updates for the ESPHome M5Stack Atom Barcode Scanner project.

## Available OTA Update Methods

The firmware supports two OTA update methods:

1. **ESPHome Dashboard**: Updates via the ESPHome dashboard's built-in OTA mechanism.
2. **HTTP OTA**: Updates via a direct HTTP URL to a firmware binary.

## Firmware Binaries

Each release publishes three firmware files to the
[GitHub Releases page](https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner/releases):

| File | Purpose |
|------|---------|
| `firmware.bin` | Main firmware — use with the ESPHome dashboard |
| `firmware.factory.bin` | Factory image — use for first-time flashing or a full chip erase |
| `firmware.ota.bin` | OTA-only image — smaller binary used for HTTP OTA updates |

## Updating via ESPHome Dashboard

1. Download `firmware.bin` from the
   [latest release](https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner/releases/latest).
2. In the ESPHome dashboard, navigate to your device.
3. Click **Upload Firmware** and select the downloaded binary.

## Updating via HTTP OTA

Use the `http_request` OTA platform to flash a binary hosted on GitHub Releases:

```yaml
ota:
  - platform: esphome
  - platform: http_request

http_request:
  verify_ssl: false

button:
  - platform: template
    name: "Update Firmware"
    on_press:
      - ota.http_request.flash:
          url: https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner/releases/latest/download/firmware.ota.bin
          verify_ssl: false
```

Change the URL to point to a specific release tag when you need a particular version, e.g.:
`…/releases/download/v2026.4.0/firmware.ota.bin`

## Choosing Which Firmware File to Flash

### Stable Channel

Download `firmware.bin` from the latest stable release tag (e.g., `v2026.4.0`).

### Beta Testing Channel

Download `firmware.bin` from the desired pre-release tag (e.g., `v2026.4.0-beta.1`).

### PR Testing

Each PR that modifies firmware or component files automatically builds a firmware artifact
posted as a comment on the PR. Download and extract the zip, then flash via the ESPHome dashboard.

### Development Channel

Compile locally from the `main` branch:

```bash
uv run esphome compile firmware/atom_lite.yaml
```

## Common OTA Issues and Solutions

### Failed Updates

If an update fails:

1. Check your device's logs for error messages.
2. Ensure your device has a stable internet connection.
3. Try updating with the ESPHome dashboard as a fallback.

### Automatic Rollbacks

If a bad update is deployed:

1. The device may roll back automatically if `safe_mode` is enabled.
2. You can manually flash an older version via the ESPHome dashboard.

### Monitoring OTA Status

The OTA update process is logged. You can monitor:

1. Download progress
2. Installation status
3. Post-update boot messages

## Advanced OTA Configuration

### Safe Mode

ESPHome's `safe_mode` component automatically boots into a recovery mode after repeated
crash loops, allowing you to re-flash the device. Add it to `firmware/atom_lite.yaml`:

```yaml
safe_mode:
  reboot_timeout: 3min
  num_attempts: 5
```

### OTA with Password Protection

Protect OTA with a password in production to prevent unauthorised updates:

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

1. **Test thoroughly**: Always test updates on a test device before deploying to production.
2. **Version control**: Keep track of which versions are running on which devices.
3. **Update windows**: Schedule updates during low-usage periods.
4. **Backup configurations**: Always keep backups of working configurations.
5. **Monitor logs**: Check logs after updates to verify successful installation.
