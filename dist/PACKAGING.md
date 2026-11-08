# Packaging (per-system .zip)

Each supported/released system gets a **.zip** containing files **specific to that system** (not a native installer format).

Example contents for `elx-sdk-0.4.1-phase4-linux-x64.zip`:
- `bin/elx` (or `elx.exe` on Windows)
- `README.md`, `LICENSE`, `LICENSE-ELX`, `LICENSE.header`
- `examples/` (`.elx` sources)
- `docs/` (optional subset)
- `VERSION.txt`, `SHA256SUMS.txt`

Do not publish a zip/binary until built and verified on that target system.
