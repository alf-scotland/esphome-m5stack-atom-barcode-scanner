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

## Notes

- Items within a tier can be worked in parallel; tiers must be completed in order.
- Each item is one branch → one PR. Do not combine items unless explicitly noted.
- When an item is completed, update its status to ✅ and note the merge commit or PR number in the table.
- If a decision is made to defer or change scope on an item, update the status to ⏸ and add a note explaining why.
