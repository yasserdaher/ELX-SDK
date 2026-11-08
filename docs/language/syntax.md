# Syntax (from observed examples/spec patterns)
- **Permissions block:** `permissions { sandbox: true, }` appears in init/examples.
- **Imports:** `import std.io`, `import std.fs`
- **Functions:** `fn main() { ... }`
- **Variables:** `let name: Type = value` (e.g., `let greeting: String = "..."`)
- **Calls:** `print(...)`, `std.fs.read_text(...)`
- **Strings:** double quotes `"..."`

> Verify against `compiler/src/lexer.c`, `parser.c`, and `ELX_SPEC.md` for authoritative syntax.
