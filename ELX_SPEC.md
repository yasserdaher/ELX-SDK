# ELX Language Specification

## Version 0.3.0-frontend

---

## 1. Bytecode Format (elbc)

The `.elbc` file is a binary container for compiled ELX programs.

### 1.1 File Header (32 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 4 | Magic | `0x45 0x4C 0x58 0x42` ("ELXB") |
| 4 | 1 | Version | Format version (currently 0x03) |
| 5 | 1 | Flags | Bit flags (0x01 = optimized, 0x02 = debug symbols) |
| 6 | 2 | Reserved | Must be zero |
| 8 | 8 | SourceSize | Size of original source in bytes |
| 16 | 4 | StringTableSize | Number of strings in string table |
| 20 | 4 | FunctionCount | Number of functions |
| 24 | 4 | WidgetCount | Number of widget declarations |
| 28 | 4 | Checksum | CRC32 of the rest of the file |

### 1.2 String Table

Following the header, the string table contains `StringTableSize` null-terminated UTF-8 strings. These include function names, type names, string literals, and identifiers referenced in the program.

### 1.3 Function Table

Each function entry:
- 4 bytes: Name index (into string table)
- 4 bytes: Parameter count
- 4 bytes: Local variable count
- 4 bytes: Instruction count
- N bytes: Instructions (variable length)

### 1.4 Widget Table

Each widget entry:
- 4 bytes: Name index (into string table)
- 4 bytes: Property count
- 4 bytes: Event count
- 4 bytes: Child widget index (or 0xFFFFFFFF for none)
- N bytes: Property entries (variable length)

### 1.5 Instruction Set

| Opcode | Name | Operands | Description |
|--------|------|----------|-------------|
| 0x00 | NOP | - | No operation |
| 0x01 | LOAD_CONST | 4-byte index | Load constant from string table |
| 0x02 | LOAD_INT | 8-byte value | Load integer literal |
| 0x03 | LOAD_FLOAT | 8-byte value | Load float literal |
| 0x04 | LOAD_BOOL | 1-byte value | Load boolean |
| 0x05 | LOAD_NIL | - | Load nil |
| 0x06 | LOAD_LOCAL | 1-byte index | Load local variable |
| 0x07 | STORE_LOCAL | 1-byte index | Store local variable |
| 0x08 | LOAD_GLOBAL | 4-byte index | Load global symbol |
| 0x09 | CALL | 2-byte arg count | Call function |
| 0x0A | RETURN | - | Return from function |
| 0x0B | BINARY_OP | 1-byte op | Arithmetic/comparison operator |
| 0x0C | UNARY_OP | 1-byte op | Logical/negation operator |
| 0x0D | JUMP | 4-byte offset | Unconditional jump |
| 0x0E | JUMP_IF_FALSE | 4-byte offset | Conditional jump |
| 0x0F | ENTER_BLOCK | - | Enter new scope |
| 0x10 | LEAVE_BLOCK | - | Leave scope |
| 0x11 | ALLOCATE | 4-byte size | Allocate memory |
| 0x12 | FREE | - | Free memory |
| 0x13 | CHECK_PERMISSION | 4-string-index | Validate permission |
| 0x14 | WIDGET_CREATE | 4-string-index | Create widget instance |
| 0x15 | WIDGET_SET_PROP | 4-string-index, 4-string-index | Set widget property |
| 0x16 | STATE_SUBSCRIBE | 4-string-index | Subscribe to state change |
| 0x17 | EVENT_BIND | 2-string-index | Bind event handler |

### 1.6 Manifest Format

The manifest is a JSON file shipped alongside the `.elbc`:

```json
{
  "version": "0.3.0",
  "name": "MyApp",
  "entry": "main",
  "permissions": {
    "sandbox": true,
    "network": ["tcp:80", "tcp:443"],
    "files": { "read": ["/etc/elvi"], "write": ["/home/user/data"] }
  },
  "capabilities": ["NetworkConnect", "FileRead"],
  "assets": ["assets/icon.png", "assets/theme.json"],
  "dependencies": [],
  "checksum": "sha256:...",
  "signed_by": null
}
```

### 1.7 Packaging

A packaged application is a directory or archive containing:
- `MyApp.elbc` — compiled bytecode
- `manifest.json` — application manifest
- `assets/` — static resources (images, styles, etc.)

---

## 2. Bytecode Format Details

### 2.1 ELF/ELBC Loader Contract (Kernel)

The kernel recognizes `.elbc` files via magic bytes. The ELBC loader:
1. Validates header magic and checksum
2. Maps string table into read-only memory
3. Allocates code segment for instructions
4. Allocates data segment for globals
5. Jumps to entry function (`main` by convention)

### 2.2 Register Convention

- Virtual registers are 64-bit
- Register 0 is hardwired to nil/void
- R1-R15 are caller-saved
- R16-R31 are callee-saved
- The stack pointer is implicit (RSP equivalent)

---

## 3. Manifest Format Details

### 3.1 Required Fields
- `name`: Application identifier (alphanumeric + underscore)
- `version`: Semantic version string
- `entry`: Entry point function name

### 3.2 Optional Fields
- `permissions`: Declared permissions (overrides code permissions)
- `capabilities`: Required capabilities
- `assets`: Static asset paths
- `dependencies`: Other ELX packages this depends on
- `checksum`: SHA-256 of the .elbc file
- `signed_by`: Signer identifier (null for unsigned)

---

## 4. Compiler Output Pipeline

```
Source (.elx) → Lexer → Token Stream → Parser → AST → Semantic Analyzer → Annotated AST
→ Optimizer → Optimized AST → Bytecode Generator → .elbc + manifest.json
```

### 4.1 Error Handling
- Lexer errors: ELX-100 to ELX-199
- Parser errors: ELX-200 to ELX-299
- Semantic errors: ELX-300 to ELX-399
- All errors include location (line, column, offset)
- The compiler continues after non-fatal errors and reports all at once
