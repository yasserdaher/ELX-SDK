# Troubleshooting
- **Build fails:** ensure gcc (C11) + make; check platform branch in Makefile.
- **Windows:** use MinGW/MSYS2; ensure `ws2_32` link works; unverified.
- **`security: unsafe path`:** CLI rejects unsafe paths.
- **Import/module errors:** std imports are partially supported; verify module availability.
