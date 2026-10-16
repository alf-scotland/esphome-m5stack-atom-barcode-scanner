# Hardware test plan

Verifies the reference firmware on a real M5Stack Atom Lite with the Atomic QR base,
end to end: the UART link, Home Assistant control of every setting, scanning in every
mode, configuration barcodes, persistence, security and factory reset. Run it on a PR's
firmware artifact before merging, and before a release.

The phases are ordered so that a failure shows up as early as possible: each phase only
relies on what the phases before it proved. Steps marked **GATE** decide whether it is
worth going on; if one fails, stop and send the log. Everything else is independent:
note the failure and carry on.

About 75 minutes, with **one power cycle** (step 7.4). Other reboots use the Restart
button.

## Setup

- The PR's `firmware.factory.bin` (from the build comment on the PR).
- The device on a USB cable to a computer with Chrome: USB powers it and
  [ESPHome Web](https://web.esphome.io/) shows its serial log whether or not it has been
  adopted. **Keep the log open for the whole test**; "log:" in a step means a line you
  should see there.
- Home Assistant open on a second screen or phone.
- The test barcodes: `D1`–`D6` are data, `C1`–`C8` configuration barcodes. Show them on a
  phone at full brightness (the scanner reads QR codes from screens well; print `D4` and
  `D5` if it struggles with 1D codes on a screen). The generated test page shows each one
  full-screen when you tap it.

The status LED: blue = host-mode scan running, purple = continuous or auto-sense mode,
green flash = barcode read, orange flash = scan timed out, orange pulse = no Wi-Fi,
white pulse = no Home Assistant connection, red pulse = setup window closed.

## 1. Flash and boot

- **1.1** Only when upgrading a device that runs a release up to 2026.7.0: in Home
  Assistant set **Volume** to **High** (a change made at runtime, to check it survives
  the upgrade). Note the other settings.
- **1.2** Flash `firmware.factory.bin` with ESPHome Web (Connect → Install). If it offers
  to erase the device, decline. Then open **Logs**.
- **1.3 GATE** Scanner link. log: `Version response (… bytes)` and `Publishing version`,
  and no `Scanner not responding`. If not: check the Atom is seated on the scanner base
  and send the log. Nothing after this works without the link.
- **1.4** Settings restore. log: `Migrating scanner settings from the previous preferences
  format` after 1.1 (no setting is sent), or `No stored scanner settings` on a fresh device
  followed by 15 `Command acknowledged`.

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
- **3.5** Click **Boot Sound** twice within a second. The switch ends in its original
  state. log: `Superseding queued command` or two acknowledged changes; never a switch
  that shows one state while the log shows the other.

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
- **4.7** Scan `D2` (UTF-8). Note exactly what **Last Barcode** shows. Home Assistant must
  stay connected. The scanner outputs GBK by default, so replacement characters (U+FFFD, shown as a diamond with a question mark) are
  a finding for a follow-up, not a failure.
- **4.8** Scan `D3` (300 characters). **Last Barcode** holds the first 255 (it ends in
  `0051-`); Home Assistant stays connected.
- **4.9** Scan `D4` (EAN-13) and `D5` (Code 128): `4006381333931` and `M5-SCAN-128`.
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
  LED blue, then green. Without a code: orange after the scan duration.
- **5.5** Pulse: same as 5.4.
- **5.6** Still in Pulse, press **Start Scan** in Home Assistant: nothing happens, the LED
  does not change. log: `Cannot start a scan outside host mode`.
- **5.7** Back to Host.

## 6. Configuration barcodes

**Config Code Scanning** is off (the default): the scanner passes configuration barcodes
on and the firmware applies them, so Home Assistant always shows the scanner's settings.
In host mode the scanner only reads after **Start Scan**. A device upgraded in 1.1 kept
its previous setting (on): turn **Config Code Scanning** off first.

- **6.1** **Start Scan**, scan `C1`. **Volume** becomes High. log: `Configuration barcode:
  buzzer_volume = high`. **Last Barcode** does not change.
- **6.2** **Start Scan**, scan `C2`: **Operation Mode** Continuous, LED purple. Scan `C3`
  (continuous mode reads it without Start Scan): back to Host.
- **6.3** **Start Scan**, scan `C4`. **Terminator** becomes CR.
- **6.4** **Start Scan**, scan `C5` (a setting the firmware does not manage). log:
  `Ignoring configuration barcode ^#SC^1020100`; no setting changes, no barcode.
- **6.5** **Start Scan**, scan `C7`. **Config Code Scanning** turns on.
- **6.6** **Start Scan**, scan `C6` (Volume Low). The scanner applies it itself now. Note:
  does **Volume** in Home Assistant change, and does the log show the code? (Expected: no
  and no. This is the drift that the default setting avoids.)
- **6.7** Turn **Config Code Scanning** off in Home Assistant, then set **Volume** to High
  to bring the scanner back in line.

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

## 9. Factory reset

Last, because it resets the scanner.

- **9.1** Enable the **Factory Reset Scanner** entity (disabled by default) and press it.
  log: `Factory reset requested`, `Command acknowledged`, a reboot, then `No stored
  scanner settings; applying all configured values`. Home Assistant shows the YAML
  values (**Volume** Medium, **Terminator** None, **Light Mode** On when reading).
- **9.2** Optional: **Start Scan** and scan `C8` (factory default). Same as 9.1.

Afterwards, set the values you want to use.

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
| `C2` | QR | `^#SC^2050204` | Continuous mode |
| `C3` | QR | `^#SC^2050208` | Host mode |
| `C4` | QR | `^#SC^3030052` | Terminator CR |
| `C5` | QR | `^#SC^1020100` | "No Read" message off (not managed; the scanner's default) |
| `C6` | QR | `^#SC^2050802` | Volume low |
| `C7` | QR | `^#SC^1040601` | Configuration code scanning on |
| `C8` | QR | `^#SC^303FFF0` | Factory default |

Configuration barcodes are from `components/m5stack_barcode/docs/AtomicQR_Reader_EN.pdf`.
Regenerate the test page with `uv run tests/hardware/make_test_page.py`.
