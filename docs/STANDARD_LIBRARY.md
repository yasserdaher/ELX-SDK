# Standard Library Reference (C API surface)

Source headers: `include/elx_std_*.h`. Runtime bindings also exist in `runtime/src/elx_stdlib.c` (registers native functions like `print`, `std.fs.*`, `std.time.*`, etc.). **Status:** PARTIALLY IMPLEMENTED — **function-by-function behavior not fully verified** — signatures below are from headers; runtime behavior must be verified per function.

## Modules
- **io** (`elx_std_io.h`): print/println/read_line/file helpers (`elx_print`, `elx_println`, `elx_file_*`, ...)
- **fs** (`elx_std_fs.h`): exists/is_file/read/write/mkdir/remove/copy/stat/list (`elx_fs_*`)
- **process** (`elx_std_process.h`): get_id/execute/spawn/wait/kill/pipes (`elx_proc_*`, `elx_exec`)
- **env** (`elx_std_env.h`): get/set/unset/home/temp/path/user/hostname (`elx_env_*`)
- **time** (`elx_std_time.h`): now/local/utc/format/sleep/elapsed (`elx_time_*`, `elx_sleep_*`)
- **str** (`elx_std_str.h`): string object helpers (`elx_str_*`)
- **list/map/set** (`elx_std_list.h`, `elx_std_map.h`, `elx_std_set.h`): container APIs
- **math** (`elx_std_math.h`): numeric functions (`elx_math_*`)
- **error** (`elx_std_error.h`): error object/codes
- **net** (`elx_std_net.h`): sockets/connect/bind/send/recv/resolve

## Runtime (ELX-level) bindings observed
`runtime/src/elx_stdlib.c` registers (examples): `print`, `printn`, `clock`, `math.*`, `str.*`, `list.*`, `file.*`, `time.now`, `process.exit`, `json.stringify`, and `std.*` aliases (`std.io.print`, `std.fs.read_text/write_text/exists/remove`, `std.process.get_id`, `std.env.get_hostname`, `std.time.to_iso8601`, etc.). Verify availability before use.

## Usage (ELX)
```elx
permissions { sandbox: true, }
import std.io
import std.fs
fn main() {
    print("Hello from ELX!")
}
```
