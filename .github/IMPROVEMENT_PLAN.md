# Improvement Plan

This document tracks all identified improvements towards making this an ESPHome core-quality component.
Update status markers as branches land. Each item maps to one branch and one PR.

**Status legend:** ⬜ not started · 🔄 in progress · ✅ done · ⏸ deferred

---

## Tier 1 — Fix before any new feature work (functional bugs + broken CI)

These are correctness issues. Nothing else should start until all of Tier 1 is merged.

| # | Status | Branch | What |
|---|--------|--------|------|
| 1 | ✅ | `fix/ci-release-failures` | Release workflow silently succeeds with no binaries (`\|\| echo` → `\|\| exit 1`); uses `secrets.GH_TOKEN` instead of built-in `github.token`; tag format not validated before `sed` |
| 2 | ✅ | `fix/rx-buffer-overflow` | `read_buffer_()` appends bytes with no size cap; `MAX_BARCODE_LENGTH` defined but never enforced in the read path |
| 3 | ✅ | `fix/command-queue-state` | `queue_command()` doesn't check whether an ACK is currently pending; timed-out commands are silently dropped with no retry or logging of which command failed |
| 4 | ✅ | `fix/nvs-preferences-robustness` | Loaded NVS enum fields not range-validated before casting; `pref_.save()` return value unchecked; no `static_assert` on struct size to catch padding changes |
| 5 | ⏸ | `fix/nvs-instance-isolation` | `fnv1_hash("m5stack_barcode")` is a constant — two component instances share the same NVS slot; must incorporate a per-instance identifier — **deferred: single-instance-per-device is the only realistic deployment** |

---

## Tier 2 — C++ quality and ESPHome patterns

| # | Status | Branch | What |
|---|--------|--------|------|
| 6 | ✅ | `refactor/actions-template-headers` | Template `play()`/`check()` bodies live in `actions.cpp` with an explicit instantiation workaround; move all bodies to `actions.h`, delete the instantiation block — `actions.cpp` becomes `TAG_ACTION` only |
| 7 | ✅ | `fix/entity-state-publishing` | `set_terminator_state()`, `set_stable_induction_time_state()`, `set_reading_interval_state()` don't call `publish_state()` after ACK; `set_light_mode_state()` and `set_scan_duration_state()` do — inconsistent |
| 8 | ✅ | `fix/version-command-queue-blocking` | `GET_VERSION` command holds the queue for up to 2 s; all other commands serialise behind it; give it a non-blocking path or make it lower priority |
| 9 | ✅ | `refactor/command-handler-constants` | `command_handlers.cpp:24` — `size_t(16)` is a magic number; extract to a named constant alongside `MAX_LOG_LENGTH` |
| 10 | ✅ | `refactor/command-handler-separation` | `StateCommand` both parses the UART response and mutates scanner state (calls `scanner->set_*_state()`); separate parsing from state mutation — handlers return parsed values via callback, scanner owns all state transitions |
| 11 | ✅ | `fix/schema-defaults` | Most optional config keys in `__init__.py` have no `default=`; add defaults matching scanner factory defaults (verify against PDFs) |

---

## Tier 3 — Firmware YAML quality

| # | Status | Branch | What |
|---|--------|--------|------|
| 12 | ⬜ | `fix/firmware-security` | Three security issues: (a) `api:` has no `encryption:` key; (b) `ota: platform: esphome` has no `password:`; (c) `http_request: verify_ssl: false` globally disables TLS — critical to fix before OTA manifest work. Also: `safe_mode: disabled: true` should be removed for reference firmware |
| 13 | ✅ | `refactor/firmware-yaml-split` | (a) Replace template operation mode select + `on_value` LED automation with the native `operation_mode_select` subcomponent; LED reactions move to confirmed scanner state events rather than YAML select value. (b) Split `firmware.yaml` into `core.yaml` (component config, scripts, common entities) + `atom_lite.yaml` (`!include core.yaml` + Atom Lite board/GPIO/LED specifics); LED status patterns (connecting, connected, scanning) borrowed from ESPHome voice module style — board-specific and live in `atom_lite.yaml`; update `release.yml` to build each device YAML independently. `firmware.yaml` becomes a symlink or stub for backwards compat during transition. Replaces item 23. |
| 14 | ⬜ | `fix/firmware-on-barcode-example` | Add `on_barcode` automation to firmware using `homeassistant.event` so the barcode value travels in the event payload — fires every scan including duplicate barcodes, no race with text sensor state. Add matching HA automation example to `README.md` showing the `esphome.barcode_scanned` trigger + `trigger.event.data.barcode` pattern (the text-sensor approach misses duplicate scans). |
| 15 | ⬜ | `fix/firmware-version-management` | `project_version` is a hardcoded string patched by `sed` in CI — fragile; switch to a `substitutions:` block so the version is defined once at the top of the file |

---

## Tier 4 — CI/CD improvements

| # | Status | Branch | What |
|---|--------|--------|------|
| 16 | ⬜ | `ci/add-cpp-static-analysis` | `lint.yml` job named `cpp-check` only runs `clang-format`; rename to `cpp-format` and add a real `cpp-lint` job running `cppcheck` (already configured in `.pre-commit-config.yaml`) |
| 17 | ⬜ | `ci/normalize-uv-sync` | `lint.yml` uses `uv sync --all-extras --dev`; firmware build workflows use `uv sync --all-extras` — inconsistency should be intentional and documented; firmware builds don't need dev deps |
| 18 | ⬜ | `ci/pin-action-shas` | All three workflows use floating tags (`actions/checkout@v4` etc.); pin to immutable commit SHAs; add Dependabot to keep them updated |
| 19 | ⬜ | `ci/add-dependabot` | No `.github/dependabot.yml` — add entries for `github-actions`, `pip` (pyproject.toml), and pre-commit hook revs |

---

## Tier 5 — Documentation cleanup

| # | Status | Branch | What |
|---|--------|--------|------|
| 20 | ⬜ | `docs/fix-contributing-guide` | `CONTRIBUTING.md` references non-existent `requirements-dev.txt` (should be `uv sync`); doesn't mention the manual `clang-tidy` pre-commit stage; doesn't include `uv run esphome compile firmware.yaml` as a verification step |
| 21 | ⬜ | `docs/fix-ota-guides` | `OTA_UPDATES_GUIDE.md` and `PRERELEASE_AND_OTA.md` reference `firmware-ota.bin` (doesn't exist; actual output is `firmware.ota.bin`); `OTA_UPDATES_GUIDE.md` describes "Git Repository OTA" not implemented anywhere — remove or implement; `PRERELEASE_AND_OTA.md` has a YAML sample with duplicate `ota:` blocks |
| 22 | ⬜ | `docs/rewrite-component-docs` | `index.rst` is LLM-generated and unreliable; full rewrite required — document every config key, action, trigger, condition, and entity; validate every claim against the PDFs and the current `__init__.py` schema |

---

## Tier 6 — ESPHome core preparation (larger efforts, work in order)

| # | Status | Branch | What |
|---|--------|--------|------|
| 23 | ⏸ | `feat/multi-device-yaml-split` | Pulled forward into item 13 — YAML split and per-device CI builds are in scope there |
| 24 | ⬜ | `feat/ota-manifest-and-update-component` | Generate a `manifest.json` per device in `release.yml` (name, version, binary URL → GitHub Release asset); publish to `gh-pages` or `main:manifests/`; add `update:` platform to firmware pointing at the manifest URL; fix `verify_ssl: false` first (item 12) |
| 25 | ⬜ | `feat/protocol-coverage-<feature>` | Audit every command in `ATOM_QRCODE_CMD_EN.pdf` against exposed HA entities/actions/conditions; one branch per gap found |
| 26 | ⬜ | `chore/esphome-core-prep` | Final pass before opening PR against esphome/esphome: verify component namespace, add `CODEOWNERS` entry, pass ESPHome's own test runner (`pytest tests/`), verify Python passes ESPHome's ruff config, verify C++ passes ESPHome's clang-tidy, document migration path away from `external_components:` |

---

## Notes

- Items within a tier can be worked in parallel; tiers must be completed in order.
- Each item is one branch → one PR. Do not combine items unless explicitly noted.
- When an item is completed, update its status to ✅ and note the merge commit or PR number in the table.
- If a decision is made to defer or change scope on an item, update the status to ⏸ and add a note explaining why.
