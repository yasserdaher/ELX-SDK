# Provenance & License Investigation (pre-publication)

## Scope
Source: repository working tree (SDK/toolchain sources, docs, examples, tests, build scripts). No full third-party code audit completed.

## Clearly attributable project files (provisional)
- Most `cli/`, `compiler/`, `core/`, `runtime/`, `std/`, `include/`, `native/`, `security/`, `platforms/`, `tests/`, `examples/`, `docs/`, `Makefile` appear as project sources in this tree. Attribution is **provisional** pending contributor/history/legal review.
- Copyright headers: see `ELX_SDK_COPYRIGHT_AUDIT.md` (ELECTRON VISION project identity; not a registered-entity claim).

## Third-party files with known licenses
- **None identified as vendored third-party source** in this audit pass (excluding system libraries). Build links system `libc`/`libm` (OS-provided). **Comprehensive scan not performed.**

## Files with unclear provenance
- Any file lacking explicit origin/author metadata; history in this working copy may be incomplete (no prior commits). **Requires human review.**

## Ownership claims needing human/legal confirmation
- “Owned by ELECTRON VISION” / exclusive ownership of all files — **not assumed**. ELECTRON VISION is a project identity; legal ownership, contributor rights, and third-party obligations require confirmation.
- Existing license terms (VOLT 1.26: `LICENSE`, `LICENSE-ELX`, `LICENSE.header`) preserved; **not modified**.

## Actions taken
- No license terms changed. No ownership claims invented. Private bridge artifact removed from working tree.
