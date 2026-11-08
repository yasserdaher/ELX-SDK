# Installation Guide (Evidence-Based)

**Version:** 0.4.0-phase4 (from `include/elx_config.h`).  
**Verified in this repo context:** Linux x64 build + `make test` passed. Other platforms: source/build files present but **not verified** here.

## Platform classification
| Platform | Status | Notes |
|---|---|---|
| Linux x64 (gcc/make) | VERIFIED | `make clean && make all && make test` passed locally |
| Debian/Ubuntu/Kali | BUILD-COMPATIBLE | Suggested: `sudo apt-get install build-essential` (verify on your system) |
| Arch | BUILD-COMPATIBLE | Suggested: `sudo pacman -S base-devel` (verify) |
| Fedora | BUILD-COMPATIBLE | Suggested: `sudo dnf groupinstall "Development Tools"` (verify) |
| macOS (Intel/Apple Silicon) | BUILD-COMPATIBLE | Requires Xcode Command Line Tools; Makefile has Darwin branch; **not tested** |
| Windows 10/11 x64 (MinGW/MSYS2) | BUILD-COMPATIBLE | Makefile has `Windows_NT` branch (`-lws2_32`, `platforms/windows/*`); **not tested** |
| Windows (Visual Studio/MSVC) | EXPERIMENTAL | Makefile is gcc-oriented; MSVC support not verified |
| ARM64 (Linux/macOS/Windows) | UNVERIFIED | Arch macros exist; no verified build/test |

## Common requirements (from Makefile)
- C compiler: `gcc` (or compatible C11)
- `make`
- Standard C library + `-lm`
- Windows: Winsock2 (`-lws2_32`) per Makefile

## Linux (verified path)
```bash
cd elx-sdk
make clean
make all
./build/elx version
./build/elx doctor
make test
```

## macOS (build-compatible, not verified)
```bash
xcode-select --install   # if needed
cd elx-sdk
make clean && make all
./build/elx version
```
If build fails, check compiler flags and platform sources in `platforms/macos/`.

## Windows 10/11 (build-compatible, not verified)
Using MSYS2/MinGW (suggested, not verified):
```bash
# In MSYS2 MinGW64 shell
pacman -S mingw-w64-x86_64-gcc make
cd elx-sdk
make clean && make all
./build/elx.exe version
```
The Makefile uses `Windows_NT` detection and links `-lws2_32`.

## Verification
```bash
./build/elx version
./build/elx doctor
./build/elx init demo
./build/elx build demo/main.elx
./build/elx run demo/main.elx
```

## Troubleshooting
- `cannot read source` / path errors: ensure paths are safe and files exist.
- Missing compiler: install `gcc`/`clang` and `make`.
- Windows link errors: ensure MinGW toolchain and `ws2_32` availability.
