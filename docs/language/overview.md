# ELX Language Overview
ELX is an experimental programming language compiled to ELBC bytecode and executed by the ELX runtime/VM. **Status:** early preview; features and behavior may change.

## Example (verified pattern)
```elx
permissions { sandbox: true, }
import std.io
fn main() {
    print("Hello from ELX!")
}
```

## Pipeline
`.elx` → compiler → `.elbc` → runtime/VM. See [compiler-and-runtime.md](compiler-and-runtime.md) and `ELX_SPEC.md`.

## Status notes
Not all specified features may be fully implemented. See [language-limitations.md](language-limitations.md).
