# Releasing

How the reference firmware is versioned and released. How devices receive releases is
described in [README.md](../README.md#updates); what the release workflow does in detail is
in [`workflows/release.yml`](workflows/release.yml).

## Versions

CalVer like ESPHome: `YYYY.MM.PATCH` (month without leading zero, patch from 0), e.g.
`2026.10.0`, `2026.10.1`. Pre-releases append `-beta.N` or `-rc.N`. The git tag is the
version prefixed with `v`.

## Stable release

1. Merge the changes to `main` (one branch and PR per change, see
   [CONTRIBUTING.md](CONTRIBUTING.md)). Test the PR's firmware artifact on hardware
   ([hardware test plan](../tests/hardware/TEST_PLAN.md)).
2. Bump `project_version` in `firmware/atom_lite.yaml` in its own
   `chore: bump version to YYYY.MM.PATCH` commit on `main`.
3. Tag that commit and push the tag:
   ```bash
   git tag vYYYY.MM.PATCH && git push origin vYYYY.MM.PATCH
   ```

The release workflow refuses a tag that is not on `main` or does not match
`project_version`. It builds the firmware, attests its provenance, publishes the GitHub
Release with generated notes and commits `manifests/atom_lite.json`, which offers the
update to every device.

Check afterwards: the release has `firmware.factory.bin` and `firmware.ota.bin`, the
manifest on `main` has the new version, and a device shows the update in Home Assistant.
If the release has a breaking change (e.g. devices must be re-adopted), add it at the top
of the release notes.

## Pre-release

Bump `project_version` to `YYYY.MM.PATCH-beta.N` (or `-rc.N`) on the commit to test (any
branch) and tag it. The workflow publishes a GitHub pre-release but does not update the
manifest, so no device is offered it; install it by hand (README, Updates).

## Hotfix

Fix on a `fix/*` branch from `main`, merge the PR, then release a new patch version as
above. Stable releases always come from `main`.
