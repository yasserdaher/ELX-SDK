# Downloads

## Policy (important)
- **No fake binaries or links.** A download is listed as available only when a verified binary/package exists.
- **Packaging format (per your requirement):** for each system, distribute a **`.zip` archive containing files specific to that system** (e.g., `elx`/`elx.exe`, README, LICENSE, examples as appropriate). Do **not** publish a “native installer format” placeholder; publish the `.zip` for that system.
- Source installation is available. Prebuilt binaries for Windows/macOS/ARM64 are **not available as verified downloads**..

## Availability matrix
| System | Arch | Status | Package (naming proposal / actual if present) | Link |
|---|---|---|---|---|
| Linux | x64 | VERIFIED (local build) | `dist/elx-sdk-0.4.0-phase4-linux-x64.zip` | Build from source (verified on Linux x64). No GitHub Release asset published in this repo commit; verify before download. |
| Windows | x64 | BUILD-COMPATIBLE (not verified here) | `elx-sdk-0.4.0-phase4-windows-x64.zip` | **Not yet available** — [Releases](https://github.com/yasserdaher/ELX-SDK/releases) / [Installation](INSTALLATION.md) |
| Windows | ARM64 | UNVERIFIED | `elx-sdk-0.4.0-phase4-windows-arm64.zip` | **Not yet available** |
| macOS | Intel | BUILD-COMPATIBLE (not verified here) | `elx-sdk-0.4.0-phase4-macos-x64.zip` | **Not yet available** |
| macOS | Apple Silicon | UNVERIFIED | `elx-sdk-0.4.0-phase4-macos-arm64.zip` | **Not yet available** |
| Linux | ARM64 | UNVERIFIED | `elx-sdk-0.4.0-phase4-linux-arm64.zip` | **Not yet available** |

## Release asset naming convention (proposal)
`elx-sdk-<version>-<os>-<arch>.zip` — each `.zip` contains files for that specific system only.

## Asset verification checklist (before marking available)
- [ ] Built on the target OS/arch (or cross-compiled with evidence)
- [ ] `elx version`/`doctor` run
- [ ] At least `init`+`build`+`run` of hello example passes
- [ ] SHA256 checksum published
- [ ] LICENSE/NOTICE included in the zip
