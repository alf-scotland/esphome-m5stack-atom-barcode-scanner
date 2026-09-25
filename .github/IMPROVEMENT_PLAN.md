# Improvement Plan

This document tracks all identified improvements for the ESPHome M5Stack Atom barcode scanner.
Update status markers as branches land. Each item maps to one branch and one PR.

**Status legend:** ⬜ not started · 🔄 in progress · ✅ done · ⏸ deferred

---

## Tier 1 — Fix before any new feature work (functional bugs + broken CI)

| # | Status | Branch | What |
|---|--------|--------|------|
| 1 | ✅ | `fix/ci-release-failures` | Release workflow silently succeeds with no binaries; uses wrong token; tag format not validated |
| 2 | ✅ | `fix/rx-buffer-overflow` | `read_buffer_()` appends bytes with no size cap; `MAX_BARCODE_LENGTH` never enforced in the read path |
| 3 | ✅ | `fix/command-queue-state` | `queue_command()` doesn't check whether an ACK is pending; timed-out commands silently dropped |
| 4 | ✅ | `fix/nvs-preferences-robustness` | NVS enum fields not range-validated; `pref_.save()` return value unchecked; no `static_assert` on struct size |
| 5 | ⏸ | — | NVS instance isolation — **permanently deferred: the M5Stack Atom hardware physically supports only one scanner connection; multi-instance deployment is not realistic** |

---

## Tier 2 — C++ quality and ESPHome patterns

| # | Status | Branch | What |
|---|--------|--------|------|
| 6 | ✅ | `refactor/actions-template-headers` | Move template `play()`/`check()` bodies to `actions.h`; `actions.cpp` becomes `TAG_ACTION` only |
| 7 | ✅ | `fix/entity-state-publishing` | Several `set_*_state()` methods didn't call `publish_state()` after ACK — inconsistent |
| 8 | ✅ | `fix/version-command-queue-blocking` | `GET_VERSION` held the queue for up to 2 s; give it a non-blocking path |
| 9 | ✅ | `refactor/command-handler-constants` | `size_t(16)` magic number in `command_handlers.cpp` — extract to named constant |
| 10 | ✅ | `refactor/command-handler-separation` | `StateCommand` both parsed UART responses and mutated scanner state — separate concerns |
| 11 | ✅ | `fix/schema-defaults` | Most optional config keys in `__init__.py` had no `default=`; add defaults matching scanner factory defaults |
| 27 | ✅ | `fix/terminator-none-barcode-truncation` | `terminator: none` caused `process_barcode_()` to fire on partial data; fixed with 20 ms idle window |
| 28 | ✅ | `fix/led-state-machine` | Seven LED state machine bugs in `atom_lite.yaml`; full rewrite of `led_off` as state dispatcher |
| 29 | ✅ | `fix/sound-command-inversion` | Sound command byte arrays were swapped relative to PDF item 11 naming |

---

## Tier 3 — Firmware YAML quality

| # | Status | Branch | What |
|---|--------|--------|------|
| 12 | ✅ | `fix/firmware-security` | Missing API encryption key, OTA password, and `verify_ssl: false` globally disabled TLS |
| 13 | ✅ | `refactor/firmware-yaml-split` | Split `firmware.yaml` into `core.yaml` + `atom_lite.yaml`; replaced template select with native `operation_mode_select` |
| 14 | ✅ | `fix/firmware-on-barcode-example` | Add `on_barcode` automation using `homeassistant.event` so barcode value travels in the event payload |
| 15 | ✅ | `fix/firmware-version-management` | `project_version` was a hardcoded string patched by `sed`; moved to `substitutions:` block |

---

## Tier 4 — CI/CD improvements

| # | Status | Branch | What |
|---|--------|--------|------|
| 16 | ✅ | `ci/add-cpp-static-analysis` | Add real `cppcheck` lint job; rename existing job from `cpp-check` to `cpp-format` |
| 17 | ✅ | `ci/normalize-uv-sync` | Normalise `uv sync` flags across workflows |
| 18 | ✅ | `ci/pin-action-shas` | Pin all GitHub Actions to immutable SHAs |
| 19 | ✅ | `ci/add-dependabot` | Add Dependabot for `github-actions`, `pip`, and pre-commit hook revs |

---

## Tier 5 — Documentation cleanup

| # | Status | Branch | What |
|---|--------|--------|------|
| 20 | ✅ | `docs/tier5-all` | Fix `CONTRIBUTING.md`: remove stale `requirements-dev.txt` reference, add clang-tidy and compile steps |
| 21 | ✅ | `docs/tier5-all` | Fix `OTA_UPDATES_GUIDE.md` and `PRERELEASE_AND_OTA.md`: wrong binary names, unimplemented features, duplicate blocks |
| 22 | ✅ | `docs/tier5-all` | Rewrite `index.rst`: LLM-generated and unreliable; document every config key, action, trigger, condition, entity |

---

## Tier 6 — Protocol coverage and quality

| # | Status | Branch | What |
|---|--------|--------|------|
| 23 | ⏸ | — | Multi-device YAML split — pulled forward into item 13 |
| 24 | ✅ | `feat/ota-manifest-and-update-component` | OTA manifest pipeline: per-device `manifests/*.json` generated on release, `update:` platform in firmware |
| 25 | ✅ | — | PDF audit complete. All 21 commands reviewed. Items 5/6 (communication method / baud rate) are N/A — changing either breaks the UART connection. Item 20 (heartbeat control) deferred — not enough user value to justify a new RX response handler. |
| 30 | ✅ | `feat/protocol-factory-reset` | PDF item 1: factory reset button — sends reset command, clears NVS, reboots so ESPHome re-applies all settings |
| 31 | ✅ | `feat/protocol-cmd-ack-sound-config-code-scan` | PDF item 14: `cmd_ack_sound_switch` — controls scanner beep on config command ACK |
| 32 | ✅ | `feat/protocol-cmd-ack-sound-config-code-scan` | PDF item 21: `config_code_scan_switch` — prevents scanner from being silently reconfigured by a scanned QR config code |
| 26 | ⏸ | — | ESPHome core submission — **permanently deferred: this repo combines firmware and component; it cannot be submitted to esphome/esphome as-is. The component is written to core quality standards and could be extracted into a standalone PR in future, but that requires a structural change outside the scope of this repo.** `.clang-tidy` is kept as a 1:1 copy of ESPHome core's config. |

---

## Tier 7 — Repository audit (2026-09)

Found with a host-platform build of the component driven by an emulated scanner
(now `tests/integration/`). All items landed on `claude/repo-audit-improvements-3376ej`.

| # | Status | What |
|---|--------|------|
| 33 | ✅ | Non-host modes never published barcodes from `loop()`: framing waited for `05 D1 …`, which is only the reply to start/stop outside host mode |
| 34 | ✅ | Start ACK and barcode in one read: barcode discarded and a false `on_scan_timeout` fired |
| 35 | ✅ | `operation_mode: continuous/auto_sense` left the scan state IDLE; mode changes updated it before the ACK |
| 36 | ✅ | Scan timeout started at queue time, not at the scanner's ACK; GET_VERSION completion relied on an "unreachable" branch |
| 37 | ✅ | 128-byte barcode cap and RX overflow publishing the tail of oversized QR codes as a barcode |
| 38 | ✅ | `scanning_binary_sensor` crashed code generation; the documented `m5stack_barcode.start: id` shorthand was rejected |
| 39 | ✅ | Entities/actions moved to core patterns (`Parented`, index-based selects, `maybe_simple_id`, UART final validation) and deduplicated with templates and tables |
| 40 | ✅ | Tests: codegen for every test YAML, enum/option consistency, host integration tests; CI compiles the test configs for ESP32 |
| 41 | ✅ | CI: script injection and secret interpolation, release cache poisoning, clang-format 17 vs 22 drift, redundant uv-lockfile workflow |
| 42 | ✅ | Firmware: dashboard adoption / `firmware.yaml` could not load the component; DLED buffer race; unlimited scan duration released the hardware trigger immediately |
| 43 | ✅ | Public release binaries embedded the API encryption key and OTA password. Now `api: encryption: {}` (per-device key provisioned at adoption, also authenticating OTA), no OTA password, a 15 min `provisioning:` window and Improv Serial; CI needs no secrets |
| 45 | ✅ | Settings changed from HA were reverted on every reboot. YAML values are now initial values: runtime changes persist, a YAML value is re-applied only when edited (per-setting YAML baseline in NVS, v2 prefs migrated) |
| 46 | ✅ | Second review: latest queued setting wins, NAK fails fast, UTF-8 sanitising and boundary-safe truncation, no optimistic entity state at boot, RX buffer size warning |
| 47 | ✅ | Third review: settings table-driven from one protocol table (each setting was wired in ~20 places); command frames checked at compile time and against the PDF in tests; commands are plain values (no callbacks or per-command heap allocation); warning status while the scanner does not answer |
| 48 | ✅ | Firmware: scan LED stuck blue when a start never began; Atom button did nothing in level/pulse mode; open fallback AP (captive portal accepts firmware uploads) no longer started on adopted devices |
| 49 | ✅ | Release: build provenance attestation, stable tags must be on `main`, `gh release` instead of a third-party action; `SECURITY.md` |
| 50 | ⏸ | Scanner state can drift from HA when settings are changed by scanning configuration barcodes (`config_code_scan_mode: enabled`, the scanner default): the documented protocol has no way to read settings back. The opcodes appear to follow Zebra's SSI protocol (C6 parameter send, D0/D1 ACK/NAK, A3/A4 revision, E4/E5 start/stop decode), whose parameter request (0xC7) might read them back — to be tried on hardware before relying on it |
| 44 | ⏸ | `pre-commit-autoupdate` PRs are opened with `GITHUB_TOKEN`, so CI does not run on them; needs a GitHub App or fine-grained PAT secret |

---

## Notes

- Items within a tier can be worked in parallel; tiers must be completed in order.
- Each item is one branch → one PR. Do not combine items unless explicitly noted.
- When an item is completed, update its status to ✅ and note the merge commit or PR number in the table.
- If a decision is made to defer or change scope on an item, update the status to ⏸ and add a note explaining why.
