# ELX Standard Library API Reference

## std.io

| Function | Description |
|----------|-------------|
| `print(format, ...)` | Print to stdout |
| `println(format, ...)` | Print to stdout with newline |
| `read_line(buffer, max)` | Read line from stdin |
| `get_char()` | Get character from stdin |
| `put_char(c)` | Put character to stdout |

## std.fs

| Function | Description |
|----------|-------------|
| `fs.exists(path)` | Check if path exists |
| `fs.read_text(path)` | Read file as text |
| `fs.write_text(path, text)` | Write text to file |
| `fs.mkdir(path)` | Create directory |
| `fs.remove(path)` | Remove file or directory |
| `fs.current_dir()` | Get current directory |
| `fs.path_join(base, rel)` | Join paths |

## std.process

| Function | Description |
|----------|-------------|
| `process.get_id()` | Get process ID |
| `process.execute(cmd)` | Execute command |
| `process.spawn(program, args)` | Spawn process |
| `process.wait(handle)` | Wait for process |

## std.env

| Function | Description |
|----------|-------------|
| `env.get(key)` | Get environment variable |
| `env.set(key, value)` | Set environment variable |
| `env.unset(key)` | Remove environment variable |
| `env.get_home()` | Home directory |
| `env.get_temp()` | Temp directory |
| `env.get_path()` | PATH variable |

## std.time

| Function | Description |
|----------|-------------|
| `time.now()` | Current timestamp |
| `time.local()` | Local time |
| `time.utc()` | UTC time |
| `time.to_iso8601(ts)` | Format as ISO 8601 |
| `time.sleep(ms)` | Sleep for milliseconds |
| `time.elapsed_ms()` | Milliseconds since epoch |

## Module System

ELX modules provide reusable, importable code units. Modules can be stdlib, local, package, or builtin types.

### APIs

| Function | Description |
|----------|-------------|
| `elx_module_cache_new()` | Create module cache |
| `elx_module_cache_free(cache)` | Free module cache |
| `elx_module_cache_add(cache, info)` | Add module to cache |
| `elx_module_cache_find(cache, name)` | Find module by name |
| `elx_module_cache_count(cache)` | Get module count |
| `elx_module_info_parse(info, path)` | Parse module manifest |
| `elx_module_info_write(info, path)` | Write module manifest |
| `elx_module_resolve(cache, name)` | Resolve module path |
| `elx_module_type_from_string(s)` | Parse type from string |
| `elx_module_type_name(type)` | Get type name string |

## Package System

ELX packages are distributable units containing modules, metadata, and dependencies.

### APIs

| Function | Description |
|----------|-------------|
| `elx_package_registry_new()` | Create package registry |
| `elx_package_registry_free(reg)` | Free registry |
| `elx_package_registry_add(reg, info)` | Add package |
| `elx_package_registry_find(reg, name)` | Find package |
| `elx_package_registry_count(reg)` | Get package count |
| `elx_package_registry_set_url(reg, url)` | Set registry URL |
| `elx_package_install(reg, name, ver)` | Install package |
| `elx_package_update(reg, name)` | Update package |
| `elx_package_remove(reg, name)` | Remove package |
| `elx_package_validate(info, err)` | Validate package |
| `elx_package_info_parse(info, path)` | Parse package manifest |
| `elx_package_info_write(info, path)` | Write package manifest |
