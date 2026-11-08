# File Formats

## `.elx` — Source code
- **Purpose:** ELX source files.
- **Created by:** developers; `elx init` generates `main.elx`.
- **Read by:** `elx build/run/test` (compiler pipeline).
- **Editable:** yes (text).
- **Examples:** `examples/hello/main.elx`, `examples/file_io/main.elx`.

## `.elbc` — ELBC bytecode
- **Purpose:** Compiled bytecode output.
- **Created by:** `elx build` (and compilation paths used by run/test).
- **Read by:** runtime/VM loader (`runtime/src/elbc_reader.c`).
- **Editable:** not intended (binary).
- **Portable/version-dependent:** may be tied to ELBC format/runtime version; treat as build artifact.
- **Spec:** see `ELX_SPEC.md` (implementation/spec alignment should be verified against source).

## `manifest.json` (project metadata)
- **Purpose:** created by `elx init` (version/name/entry/target/permissions).
- **Editable:** yes (JSON).

No other ELX-specific extensions are claimed without code evidence.
