# Branching and Release Strategy

This document outlines the branching and release strategy for the ESPHome M5Stack Atom Barcode Scanner project.

## Branching Model

We follow a simplified GitFlow approach with the following branches:

### Main Branches

- **`main`**: The primary development branch containing the latest stable code. All production builds and releases are sourced from this branch.

### Support Branches

- **`feature/*`**: Feature branches for developing new features.
- **`bugfix/*`**: Branches specifically dedicated to fixing bugs.
- **`release/*`**: Release preparation branches, used to prepare and finalize releases.
- **`release/*-beta.*`** or **`release/*-rc.*`**: Pre-release branches for beta and release candidate testing.
- **`hotfix/*`**: Emergency fix branches that branch directly from tagged releases to fix critical issues.

## Workflow

### Feature Development

1. Create a new feature branch from `main`:
   ```bash
   git checkout main
   git pull
   git checkout -b feature/my-new-feature
   ```

2. Develop the feature, making regular commits.

3. When ready, create a pull request against `main`.

4. After code review and all CI checks pass, the feature can be merged.

### Bug Fixes

1. Create a new bugfix branch from `main`:
   ```bash
   git checkout main
   git pull
   git checkout -b bugfix/issue-description
   ```

2. Fix the bug, making sure to include tests where appropriate.

3. Create a pull request against `main`.

4. After code review and CI checks, the fix can be merged.

### Release Process

1. When ready for a release, create a release branch:
   ```bash
   git checkout main
   git pull
   git checkout -b release/YYYY.MM.0
   ```

2. Update version numbers in `firmware.yaml`:
   ```yaml
   project_version: "YYYY.MM.0"
   ```

3. Complete any final testing and bug fixes directly in the release branch.

4. When the release is ready, merge back to `main`:
   ```bash
   git checkout main
   git merge --no-ff release/YYYY.MM.0
   ```

5. Tag the release:
   ```bash
   git tag vYYYY.MM.0
   git push origin vYYYY.MM.0
   ```

6. The CI system will automatically build and publish the release.

### Pre-release Process

For beta and release candidate testing:

1. Create a pre-release branch from `main` or an existing `release/*` branch:
   ```bash
   git checkout main
   git pull
   git checkout -b release/YYYY.MM.0-beta.1
   ```

2. Update version in `firmware.yaml`:
   ```yaml
   project_version: "YYYY.MM.0-beta.1"
   ```

3. Tag the pre-release:
   ```bash
   git tag vYYYY.MM.0-beta.1
   git push origin vYYYY.MM.0-beta.1
   ```

4. The CI system will automatically build and publish the pre-release with OTA-enabled firmware.

5. After testing is complete and issues are fixed, create the final release or the next pre-release as needed.

For more details on pre-releases and OTA updates, see [PRERELEASE_AND_OTA.md](PRERELEASE_AND_OTA.md).

### Hotfixes

1. If a critical bug is found in a release, create a hotfix branch from the release tag:
   ```bash
   git checkout vYYYY.MM.0
   git checkout -b hotfix/critical-bug-description
   ```

2. Fix the issue and update the patch version:
   ```yaml
   project_version: "YYYY.MM.1"
   ```

3. Create a pull request against `main`.

4. After merging, tag the new release:
   ```bash
   git tag vYYYY.MM.1
   git push origin vYYYY.MM.1
   ```

## Versioning Scheme

We follow ESPHome's versioning scheme, which uses a CalVer approach:

**`YYYY.MM.PATCH`**

- **YYYY**: Year of the release (e.g., 2024)
- **MM**: Month of the release (e.g., 06 for June)
- **PATCH**: Incremental number for patches/hotfixes (starting at 0)

### Examples

- `2024.3.0`: First release in March 2024
- `2024.3.1`: First patch to the March 2024 release
- `2024.3.2`: Second patch to the March 2024 release
- `2024.4.0`: First release in April 2024

### Pre-releases

For pre-releases, we append a beta or release candidate identifier:

- `2024.3.0-beta.1`: First beta of the March 2024 release
- `2024.3.0-beta.2`: Second beta of the March 2024 release
- `2024.3.0-rc.1`: First release candidate of the March 2024 release

Pre-releases are tagged in Git with a 'v' prefix (e.g., `v2024.3.0-beta.1`) and are marked as pre-releases on GitHub.

## Release Frequency

- Regular releases: Monthly (if there are significant changes)
- Patches/hotfixes: As needed for critical bugs
- Pre-releases: As needed during development cycles, especially before major changes

## Compatibility Guarantees

- Breaking changes will be clearly documented in release notes
- Backwards compatibility with ESPHome will be maintained where possible
- Major breaking changes will be announced in advance
- Pre-releases may contain breaking changes or experimental features
