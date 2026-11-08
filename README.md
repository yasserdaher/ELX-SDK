# ELX — Programming Language & Developer SDK

<p align="center">
  <strong>ELX SDK</strong> — Developer toolchain for the ELX programming language<br/>
  Published by <strong>ELECTRON VISION</strong> (project identity)
</p>

<p align="center">
  <a href="README.en.md">English</a> · <a href="README.ar.md">العربية</a>
</p>

<p align="center">
  <img alt="Version" src="https://img.shields.io/badge/version-0.4.0--phase4-2E8EFF" />
  <img alt="License" src="https://img.shields.io/badge/license-VOLT%201.26-7B61FF" />
  <img alt="Linux" src="https://img.shields.io/badge/Linux%20x64-VERIFIED-2ea44f" />
  <img alt="macOS" src="https://img.shields.io/badge/macOS-BUILD--COMPATIBLE-lightgrey" />
  <img alt="Windows" src="https://img.shields.io/badge/Windows-BUILD--COMPATIBLE-lightgrey" />
</p>

## Overview

ELX is an experimental programming language toolchain and SDK. This repository contains the public developer-facing SDK components: CLI, compiler integration, runtime/VM, ELBC bytecode tooling, standard library interfaces, native/security modules, diagnostics, formatting/linting/debugging utilities, module/package/spec tools, examples, and tests.

- **Repository:** https://github.com/yasserdaher/ELX-SDK
- **Version (source of truth):** `ELX_SDK_VERSION "0.4.0-phase4"` (`include/elx_config.h`)
- **Status:** EXPERIMENTAL / early developer preview (evidence-based; see [DOCUMENTATION_AUDIT.md](DOCUMENTATION_AUDIT.md))

## Quick Start (Linux x64 — verified)

```bash
cd elx-sdk
make clean && make all
./build/elx version
./build/elx init hello_app
./build/elx build hello_app/main.elx
./build/elx run hello_app/main.elx
```

## Documentation

- [English README](README.en.md) · [Arabic README](README.ar.md)
- [Docs index](docs/README.md) · [الفهرس العربي](docs/ar/README.md)
- [Installation](docs/INSTALLATION.md) · [CLI Reference](docs/CLI_REFERENCE.md)
- [Language](docs/language/overview.md) · [Standard Library](docs/STANDARD_LIBRARY.md)
- [File Formats](docs/FILE_FORMATS.md) · [Architecture](docs/ARCHITECTURE.md)
- [Downloads](docs/DOWNLOADS.md)

## Downloads

Prebuilt binaries are provided **only when verified**. Each system package is distributed as a **`.zip` archive containing files specific to that system** (not a native installer format). See [docs/DOWNLOADS.md](docs/DOWNLOADS.md).

| System | Status | Package |
|---|---|---|
| Linux x64 | VERIFIED (local build) | `dist/elx-sdk-0.4.0-phase4-linux-x64.zip` (when built) |
| Windows x64 | BUILD-COMPATIBLE (source present, not tested here) | Not yet available as verified binary |
| macOS | BUILD-COMPATIBLE (source present, not tested here) | Not yet available as verified binary |
| ARM64 variants | UNVERIFIED | Not yet available |

Source installation is the verified path on Linux; other platforms require local build and testing.

## License

Licensed under the **VOLT 1.26 License Agreement**. See [LICENSE](LICENSE), [LICENSE-ELX](LICENSE-ELX), and [LICENSE.header](LICENSE.header). License terms are preserved as provided.

## Contributing / Security

See [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), [SECURITY.md](SECURITY.md), and [CHANGELOG.md](CHANGELOG.md).
