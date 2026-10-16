# Security Policy

## Supported versions

Only the latest release of the reference firmware and the component on `main` receive
security fixes. Devices running the reference firmware are offered the fix through the
**Firmware Update** entity in Home Assistant.

## Reporting a vulnerability

Please report vulnerabilities privately through GitHub:
**Security → Report a vulnerability** on this repository. Do not open a public issue.

Include the affected version, how to reproduce the problem and its impact. You can expect
an acknowledgement within a week.

## Verifying a firmware binary

Release binaries carry a build provenance attestation. To check that a binary was built
by this repository's release workflow:

```bash
gh attestation verify firmware.ota.bin --repo alf-scotland/esphome-m5stack-atom-barcode-scanner
```

## Security model

The published firmware contains no credentials. Each device receives its own API
encryption key when it is adopted; that key also authenticates ESPHome OTA uploads. See
[README.md](../README.md#using-the-reference-firmware) for the setup window, the fallback
access point and how Wi-Fi is changed on an adopted device.
