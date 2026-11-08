# Changelog
## 0.4.1-phase4 — 2026-11-09

### Fixed
- **ELBC 3.2 compiler/runtime compatibility:** `emit_program()` no longer writes unframed widget/state opcode streams into the module body. The runtime reader does not consume module-level widget/state instruction bytes; the previous behavior caused reader/writer misalignment (`elx_module_load` failure, e.g. ELX-Calculator). `widget_count` remains a header metadata field; widget reconstruction requires a framed section in a future format revision.
- **ELBC reader hardening:** bounds validation for function `code_size`, string pool/import/export counts, export `function_index`, truncation, and allocation failures. Added `elx_module_load_error()` and detailed diagnostics (`elx_sdk_run`/runtime report the failing section).
- **Checksum handling:** CRC32 mismatches report the stored and computed values and are rejected by default. Loading a mismatched file is permitted only when the explicit compatibility option `ELX_ELBC_ALLOW_BAD_CRC=1` is enabled.


### Added
- `tests/test_elbc.c`: round-trip compile/load/run, imports/strings, multi-function, existing fixtures, corruption cases (bad magic/version/oversized code/truncation), checksum-mismatch compatibility.

### Verified
- `make clean && make all`, `make test`, `./build/test_elbc` (25 passed, 0 failed), `elx doctor`, ELX-Calculator compiles and module loads (no `main` entry point in app — expected for current source).

## 0.4.1-phase4
- Public SDK documentation and packaging improvements (evidence-based).
- CLI/compiler/runtime/stdlib as present in source; Linux x64 build+test verified in development context.
