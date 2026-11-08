# CLI Reference (elx)

Source of truth: `cli/elx.c` and `./build/elx help` (v0.4.0-phase4).

## Global
```text
ELX SDK v0.4.0-phase4
Commands:
  build <file.elx> [--target <t>] [-o <out>]
  run <file.elx> [--target <t>]
  test <file.elx> [--target <t>]
  init <name>
  doctor
  version
  help
  module list
  module info <name>
  module resolve <name>
  package install <name> [--version <v>]
  package update <name>
  package remove <name>
  package list
  package search <query>
  diag <file> [--level error|warning|info]
  fmt <file>
  lint <file> [--strict]
  debug <file> [--break <file:line>]
  freeze lock [--version <v>]
  freeze unlock
  freeze status
  freeze report
  compat check
  compat graph
  compat report
  rc
  stabilize
  release
```

## Verified commands (Linux x64)
| Command | Example | Expected (observed) |
|---|---|---|
| `version` | `./build/elx version` | `elx 0.4.0-phase4 (core 0.4.0-phase4, sdk 0.4.0-phase4)` |
| `doctor` | `./build/elx doctor` | Prints environment; ends with `All checks passed!` on verified setup |
| `init` | `./build/elx init demo` | Creates `demo/main.elx` + `demo/manifest.json` |
| `build` | `./build/elx build demo/main.elx` | Compiles to `.elbc`; reports success |
| `run` | `./build/elx run demo/main.elx` | Compiles/runs; prints `Hello from ELX!`, exit 0 for template |
| `test` | `./build/elx test demo/main.elx` | Integration behavior per CLI (see `make test`) |

## Verification status
- **Verified (Linux x64, integration flow):** `version`, `doctor`, `init`, `build`, `run`, `test` (via `make test` / basic usage).
- **Exposed but not individually verified in this audit:** `module *`, `package *`, `diag`, `fmt`, `lint`, `debug`, `freeze *`, `compat *`, `rc`, `stabilize`, `release`. Treat as **PARTIALLY IMPLEMENTED / UNVERIFIED** until tested.

## Notes
- Paths are validated (`elx_sec_is_path_safe`).
- Default target: `elvi` (CLI variable); `--target` accepted by build/run/test.
- `module/package/diag/fmt/lint/debug/freeze/compat/rc/stabilize/release` are exposed by CLI; implementation completeness is **PARTIALLY IMPLEMENTED / EXPERIMENTAL** — verify behavior on your use case.
