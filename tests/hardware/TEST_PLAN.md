# Hardware test plan

Verifies the reference firmware on a real M5Stack Atom Lite with the Atomic QR base,
end to end: the UART link, Home Assistant control of every setting, scanning in every
mode, configuration barcodes, persistence and security. Run it on a PR's firmware
artifact before merging, and before a release.

The phases are ordered so that a failure shows up as early as possible: each phase only
relies on what the phases before it proved. Steps marked **GATE** decide whether it is
worth going on; if one fails, stop and send the log. Everything else is independent:
note the failure and carry on.

About 60 minutes, with **one power cycle** (step 7.4). Other reboots use the Restart
button.

## Setup

- The PR's `firmware.factory.bin` (from the build comment on the PR).
- The device on a USB cable to a computer with Chrome: USB powers it and
  [ESPHome Web](https://web.esphome.io/) shows its serial log whether or not it has been
  adopted. **Keep the log open for the whole test**; "log:" in a step means a line you
  should see there.
- Home Assistant open on a second screen or phone.
- The test barcodes: `D1`–`D6` are data, `C1`–`C3` configuration barcodes. Show them on a
  phone at full brightness (the scanner reads QR codes from screens well; print `D4` and
  `D5` if it struggles with 1D codes on a screen). The generated test page shows each one
  full-screen when you tap it.

The status LED: blue = host-mode scan running, purple = continuous or auto-sense mode,
yellow flash = the scanner decoded a code, green flash = barcode read, orange flash = scan
timed out, orange pulse = no Wi-Fi, white pulse = no Home Assistant connection, red pulse =
setup window closed.

## 1. Flash and boot

- **1.1** Only when upgrading a device that runs a release up to 2026.7.0: in Home
  Assistant set **Volume** to **High** (a change made at runtime, to check it survives
  the upgrade). Note the other settings.
- **1.2** Flash `firmware.factory.bin` with ESPHome Web (Connect → Install). If it offers
  to erase the device, decline. Then open **Logs** and press **Reset Device**: the log
  only shows a whole boot from there.
- **1.3 GATE** Scanner link. log: `Version response (… bytes)` and `Publishing version`,
  and no `Scanner not responding`. If not: check the Atom is seated on the scanner base
  and send the log. Nothing after this works without the link.
- **1.4** Settings restore. log: `Restoring scanner settings from flash` and no `Queuing`
  of a setting: the first boot after flashing, before the log was open, stored them.

## 2. Wi-Fi and adoption

Do this within 15 minutes of step 1.2, which restarted the setup window.

- **2.1** In ESPHome Web choose **Connect to Wi-Fi** (Improv Serial). LED: orange pulse,
  then white pulse once Wi-Fi is up.
- **2.2 GATE** Adoption. When upgrading, first delete the old device in Home Assistant.
  Add the discovered device: Home Assistant must not ask for an encryption key. LED:
  green for a second, then off. log: an API client connects.
- **2.3 GATE** State sync. Every setting entity shows a value within 10 s (only **Last
  Barcode** stays unknown). After 1.1, **Volume** is still **High**.
- **2.4** **Firmware Update** shows the PR build as installed and may offer the latest
  release as an update. Do not install it: it would replace the PR build.

## 3. Control from Home Assistant

- **3.1** Set **Volume** to Low, Medium, High. Each change: one beep, louder each time.
  log: `Command acknowledged` before the entity changes.
- **3.2** Turn **Command ACK Sound** off, change **Volume**: no beep. Turn it back on.
- **3.3** **Light Mode** Always on: the white floodlight stays lit. Always off: dark.
  Back to On when reading.
- **3.4** **Locate Light Mode** Always on: the aiming light stays lit. Back to On when
  reading.

## 4. Scanning in host mode

- **4.1 GATE** Press **Start Scan**. LED blue, **Scanning** on, scanner light on. Scan
  `D1`. LED green flash, beep, **Last Barcode** `HELLO-123`, **Barcode Scanned** fires,
  **Scanning** off.
- **4.2** Press the Atom's button, scan `D6`. **Last Barcode** `SECOND-CODE`.
- **4.3** Press **Start Scan** and scan nothing. After 3 s: LED orange flash, **Scanning**
  off. log: `Scan timed out after 3000 ms`.
- **4.4** **Start Scan**, then **Stop Scan** at once. LED off, **Scanning** off, scanner
  light off, no timeout afterwards.
- **4.5** **Decode Sound** off: scanning `D1` is silent. **Decoding Success Light** off: no
  flash on the scanner when it reads. **Sound** off: no beep at all, including setting
  changes. Turn all three back on.
- **4.6** **Scan Duration** 1s: 4.3 times out after 1 s. Unlimited: no timeout within
  30 s; **Stop Scan**. Back to 3s.
- **4.7** Scan `D2` (UTF-8). Home Assistant stays connected. The scanner outputs GBK by
  default, so **Last Barcode** shows replacement characters (a diamond with a question
  mark) and the log `replaced … invalid sequence(s)`; 6.2 switches it to UTF-8.
- **4.8** Scan `D3` (300 characters). **Last Barcode** holds the first 255 (it ends in
  `0051-`); Home Assistant stays connected.
- **4.9** Scan `D4` (EAN-13) and `D5` (Code 128): `4006381333931` and `M5-SCAN-128`. If
  they are not read from the screen, print them; note whether the LED flashed yellow.
- **4.10** For each **Terminator** (CR LF, CR, TAB, CR CR, CR LF CR LF, None): set it,
  scan `D1`. **Last Barcode** is exactly `HELLO-123` every time, with nothing after it.

## 5. Other operation modes

- **5.1** **Operation Mode** Continuous: LED purple, **Scanning** on. Hold `D1` in front:
  published, then again no sooner than **Same Code Interval** (500 ms). Set it to 2s:
  repeats about every 2 s. Alternate `D1` and `D6`: each is published at once.
- **5.2** **Reading Interval** 2s: noticeably slower reads. Back to 500ms.
- **5.3** Auto-Sense: LED purple. Bring `D1` into view: read. **Stable Induction Time** 1s:
  the read comes noticeably later. Back to 500ms.
- **5.4** Level: LED off, **Scanning** off. Press the Atom's button with `D1` in view:
  LED blue, then green. Press it again right away: blue and green again (the log never
  shows `already running`). Without a code: orange after the scan duration.
- **5.5** Pulse: same as 5.4.
- **5.6** Still in Pulse, press **Start Scan** in Home Assistant: nothing happens, the LED
  does not change. log: `Cannot start a scan outside host mode`.
- **5.7** Back to Host.

## 6. Configuration barcodes

With **Config Code Scanning** off (the default) the scanner ignores configuration barcodes,
so they cannot change a setting behind Home Assistant's back. With it on, the scanner
applies them itself without telling the firmware. A device upgraded in 1.1 kept its
previous setting (on): turn **Config Code Scanning** off first. In host mode the scanner
only reads after **Start Scan**.

- **6.1** Set **Volume** to Low. **Start Scan**, scan `C1` (Volume high): LED yellow, then
  orange, nothing in the log. Change **Terminator** and back: the confirmation beeps are
  still low.
- **6.2** Turn **Config Code Scanning** on. **Start Scan**, scan `C2` (UTF-8 output): the
  scanner beeps. **Start Scan**, scan `D2`: **Last Barcode** is exactly `Grüße ÄÖÜ 🙂`.
- **6.3** **Start Scan**, scan `C1`. Note whether **Volume** in Home Assistant changes and
  whether the log shows the code (expected: no and no; the scanner now beeps loud).
- **6.4** Turn **Config Code Scanning** off, press **Resend Scanner Settings**: 15
  `Command acknowledged`, and the confirmation beeps are low again, as **Volume** shows.
  The scanner keeps UTF-8 output.

## 7. Persistence

- **7.1** Set distinct values: **Volume** High, **Terminator** CR, **Light Mode** Always
  off, **Scan Duration** 5s, **Boot Sound** on.
- **7.2** Press **Restart**. After it reconnects every entity shows the same values at
  once. log: `Restoring scanner settings from flash` and no setting commands sent.
- **7.3** Only if the device is adopted in your ESPHome dashboard: install the same PR
  build over the air from there. Same values afterwards.
- **7.4 GATE** **Power cycle**: unplug the USB cable for 10 s and plug it back in. The
  scanner beeps at power-up (it kept Boot Sound). LED: orange pulse, white pulse, green,
  off. Values unchanged; scanning `D1` beeps at high volume, floodlight stays off. If the
  scanner behaves like its factory defaults while Home Assistant shows these values, the
  scanner does not keep settings over a power loss: stop and report.

## 8. Security

- **8.1** Switch off your Wi-Fi access point (or take the device out of range) for 3
  minutes. LED orange pulse. Your phone's Wi-Fi list must **not** show an
  `atom-barcode-…` network. Switch the access point back on: the device reconnects on
  its own.

## If the scanner stops answering

The log shows `Scanner not responding` for every command, also after a power cycle. The
scanner's own factory defaults (its factory default barcode, scanned while **Config Code
Scanning** is on) switch its output to USB. It still receives commands, so the firmware's
settings reach it at boot, including Config Code Scanning off and sound off. To switch it
back:

1. Turn **Config Code Scanning** on. The switch stays off (the answer goes to USB), but the
   scanner applies it.
2. Hold `C3` in front of the scanner and press **Hardware Trigger** (Diagnostic). The LED
   flashes yellow when it reads the code.
3. Press **Resend Scanner Settings**: every setting is sent and acknowledged.

## Barcodes

| Label | Type | Content | Purpose |
|---|---|---|---|
| `D1` | QR | `HELLO-123` | Plain data |
| `D2` | QR | `Grüße ÄÖÜ 🙂` | UTF-8 |
| `D3` | QR | `0001-0002-…-0060-` (300 characters) | Truncation to 255 |
| `D4` | EAN-13 | `4006381333931` | 1D |
| `D5` | Code 128 | `M5-SCAN-128` | 1D |
| `D6` | QR | `SECOND-CODE` | A second code |
| `C1` | QR | `^#SC^2050800` | Volume high |
| `C2` | QR | `^#SC^3030062` | UTF-8 output ("Unicode" data encoding) |
| `C3` | QR | `^#SC^3030010` | Serial port output ("TTL 232"); recovery only |

Configuration barcodes are from `components/m5stack_barcode/docs/AtomicQR_Reader_EN.pdf`.
Regenerate the test page with `uv run tests/hardware/make_test_page.py`.
