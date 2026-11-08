# SDK API Reference (Public Headers)

Inspect `include/*.h` for authoritative signatures. Key headers:
- `elx_config.h` — version/platform/arch macros
- `elx_core.h` — core compile/run types and functions
- `elx_sdk_security.h`, `elx_native.h` — security/native interfaces
- `elx_diagnostics.h`, `elx_formatter.h`, `elx_linter.h`, `elx_debugger.h` — tooling APIs
- `elx_module.h`, `elx_package.h`, `elx_spec_freeze.h`, `elx_compat.h`, `elx_elx1.h` — module/package/spec APIs
- `elx_std_*.h` — standard library C APIs (see [STANDARD_LIBRARY.md](STANDARD_LIBRARY.md))

**Stability:** UNVERIFIED unless explicitly tested. Do not rely on undocumented internal symbols.
