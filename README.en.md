# ELX SDK (English)

## Project identity
- **Name:** ELX — Programming Language & Developer SDK
- **Publisher:** ELECTRON VISION (project identity; not claimed as a registered legal entity)
- **Version:** 0.4.0-phase4 (`ELX_SDK_VERSION`, `include/elx_config.h`)
- **Status:** EXPERIMENTAL / early developer preview
- **Repository:** https://github.com/yasserdaher/ELX-SDK
- **License:** VOLT 1.26 License Agreement — see [LICENSE](LICENSE), [LICENSE-ELX](LICENSE-ELX)

## Feature catalog (evidence-based)
| Component | Status | Notes |
|---|---|---|
| CLI (`elx`) | IMPLEMENTED | Commands: build/run/test/init/doctor/version/help/module/package/diag/fmt/lint/debug/freeze/compat/rc/stabilize/release (see `cli/elx.c`) |
| Compiler pipeline | PARTIALLY IMPLEMENTED | lexer/parser/semantic/optimizer/elbc sources present; behavior verified via integration tests/examples on Linux |
| Runtime/VM | PARTIALLY IMPLEMENTED | vm/stack/arena/elbc_reader/runtime/stdlib present; executes compiled .elbc |
| ELBC bytecode | IMPLEMENTED (format in use) | `.elbc` produced by build; spec in `ELX_SPEC.md` |
| Standard library (C headers + runtime bindings) | PARTIALLY IMPLEMENTED | io/fs/process/env/str/list/map/set/math/error/net/time modules present; not all APIs fully verified |
| Native interfaces | EXPERIMENTAL | `native/` present |
| Security components | EXPERIMENTAL | `security/` present; sandbox flag used in examples; no security certification claimed |
| Diagnostics/formatter/linter/debugger | PARTIALLY IMPLEMENTED | modules present; CLI exposes commands; behavior depends on implementation completeness |
| Module/package/spec/compat tools | EXPERIMENTAL | CLI and core modules present |
| Examples & tests | IMPLEMENTED (Linux) | `examples/` (hello,file_io,multi_file,process), `make test` integration, `tests/test_std.c` |

## Quick start (Linux x64 — VERIFIED)
```bash
make clean && make all
./build/elx doctor
./build/elx version
./build/elx init hello_app
./build/elx build hello_app/main.elx
./build/elx run hello_app/main.elx
```
Expected: prints "Hello from ELX!" and exits 0 (based on init template).

## Platform support (evidence-based)
| OS | Status | Evidence |
|---|---|---|
| Linux x64 | VERIFIED | Local build + `make test` passed in this environment |
| Windows 10/11 x64 | BUILD-COMPATIBLE | Source/Makefile/platforms present; no verified binary/test here |
| macOS (Intel/Apple Silicon) | BUILD-COMPATIBLE | Source/Makefile/platforms present; no verified binary/test here |
| Debian/Ubuntu/Kali/Arch/Fedora | BUILD-COMPATIBLE (Linux family) | Likely buildable with gcc/make deps; not individually verified |
| ARM64 | UNVERIFIED | Arch macros exist; no verified build/test |

## Limitations
- No prebuilt binaries claimed except when verified and packaged per system.
- Some CLI subcommands expose modules that may be incomplete; verify behavior before relying on them.
- No fake performance, security, or compatibility claims.

## Links
- [Installation](docs/INSTALLATION.md) · [CLI](docs/CLI_REFERENCE.md) · [Language](docs/language/overview.md) · [Stdlib](docs/STANDARD_LIBRARY.md) · [Architecture](docs/ARCHITECTURE.md) · [Downloads](docs/DOWNLOADS.md)
