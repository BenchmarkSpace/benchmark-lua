# Lua for Flight Deck

This fork of [lua/lua](https://github.com/lua/lua) carries the Flight Deck build of the Lua
interpreter. The `flightdeck` branch tracks upstream tag `v5.4.9`
(`312b9efaa1061c2c4cad08554dbc1351c3270eef`).

## No vendored file is edited

Everything this fork adds sits in files upstream does not own:

| Path | Purpose |
|---|---|
| `CMakeLists.txt` | Builds the `lua` static library for the target. |
| `flightdeck/luaconf_flightdeck.h` | Configuration, force-included ahead of every translation unit. |
| `flightdeck/fd_noparser.c` | The two symbols the source compiler would otherwise provide. |
| `FLIGHTDECK.md` | This file. |

Every configuration knob used is one upstream already guards with `#if !defined(...)`, so the
header supplies a value before Lua's own default is reached and nothing has to be patched. That
keeps `git merge upstream/master` free of conflicts in Lua's own sources.

```
git remote add upstream https://github.com/lua/lua.git   # once
git fetch upstream --tags
git merge v5.4.10                                        # when the time comes
```

After a merge, rerun the size and link checks below and re-verify that
`luaY_parser` and `luaU_dump` are still the only compiler symbols the remaining sources
reference.

## What the target build contains

**The source compiler is absent.** `lparser.c`, `lcode.c` and `ldump.c` are not built. Scripts
are compiled on a development machine and reach the device as binary chunks. This removes the
compiler from the image and removes the transient memory a parse would need — a parse costs
roughly two to four times the source size, which is significant against a 24 KB arena.

Two symbols in the remaining sources still reference the compiler, and `flightdeck/fd_noparser.c`
defines both:

- `luaY_parser`, reached from `f_parser` in `ldo.c` when a chunk does not begin with
  `LUA_SIGNATURE`. It throws `LUA_ERRSYNTAX`.
- `luaU_dump`, reached from `lua_dump` in `lapi.c`. It reports failure.

Both are called inside a protected call, so they surface as ordinary Lua errors. The scripts
component additionally passes mode `"b"` to `lua_load`, which makes `ldo.c` reject a text chunk
in `checkmode` before `luaY_parser` is reached; the stub is the backstop for any path that does
not set a mode.

**Only the permitted libraries are built.** `lauxlib`, `lbaselib`, `lmathlib`, `lstrlib` and
`ltablib`. Absent: `liolib`, `loslib`, `loadlib`, `ldblib`, `lcorolib`, `lutf8lib`. `linit.c` is
absent too — the scripts component opens libraries itself, so the permitted set is stated in one
place rather than two.

**`llex.c` is still built.** `luaX_init` is called from `f_luaopen` and interns and fixes the
`_ENV` name, which loaded bytecode and the error paths in `ldebug.c` rely on. A reduced stub
would reclaim about 4 KB but changes runtime string interning behaviour, so it is left alone.

## Configuration

`flightdeck/luaconf_flightdeck.h` sets:

| Knob | Value | Why |
|---|---|---|
| `LUA_32BITS` | 0 (upstream default, not set here) | `lua_Integer` is 64-bit and `lua_Number` is a `double`, so `uid` and other 64-bit attributes round-trip exactly. The SAMV71 FPU is double precision (`-mfpu=fpv5-d16`), so doubles are hardware backed. The cost is a 16-byte `TValue` rather than 8. |
| `LUAI_MAXCCALLS` | 60 | Bounds nested reentrant calls into the VM. Lua-to-Lua calls reuse the interpreter's C frame and do not count, so this bounds C stack use rather than script call depth. |
| `luai_makeseed` | `fd_lua_seed()` | Keeps `<time.h>` out of the build and makes the string-hash seed reproducible for a given boot. |
| `lua_writestring` / `lua_writeline` / `lua_writestringerror` | Flight Deck log | `print()` and the warning and panic paths reach the log component instead of stdout and stderr. |

The library therefore has exactly three undefined symbols the firmware must supply:

```
fd_lua_seed        unsigned int fd_lua_seed(void);
fd_lua_write       void fd_lua_write(const char *s, unsigned long len);
fd_lua_write_error void fd_lua_write_error(const char *fmt, const char *param);
```

They are implemented in `sdk/components/scripts/`. Defining `LUA_FD_HOST` keeps the upstream
stdio behaviour, for host tools built from the same sources.

## Size

Measured on `cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -Os -ffunction-sections
-fdata-sections`, linked with `--gc-sections` against a consumer that exercises `luaL_newstate`,
`lua_load` and `lua_pcall`. Toolchain gcc 13.2.1; the project builds with 10.3, so expect a few
percent of drift.

| Build | Flash |
|---|---|
| This configuration, with the project's security flags | 80,788 B |
| This configuration, without them | 72,186 B |
| Full standard Lua, all libraries plus the on-target compiler, for reference | 192,646 B |

The project's `-fstack-protector-strong`, `-fstack-clash-protection` and `_FORTIFY_SOURCE=3`
therefore cost **8,602 bytes, or 11.9 %**, on this library. They are kept: stack protection on
the interpreter that executes operator-supplied code is where it is most worth having.

Newlib's floating point formatting (`_vfprintf_r`, `_svfprintf_r`, `_dtoa_r`) is already linked
into the firmware, so Lua's number to string conversion adds nothing on that front.

Further trims available and not yet taken: string patterns in `lstrlib.c` (~6 KB), a reduced
`luaX_init` (~4 KB), `table.sort` (~2 KB).

## Building

The library is added by the deployment's `CMakeLists.txt` alongside the other libraries:

```cmake
add_subdirectory(${LIBRARIES}/benchmark-lua build/lua)
...
target_link_libraries(${PROJECT_NAME} PUBLIC libfd fd lua freertos_kernel asf csp lfbb littlefs bson)
```

`-ffunction-sections -fdata-sections` are set on this target, but `--gc-sections` is a link
option and belongs to the application. Without it the unreferenced parts of the permitted
libraries are still linked in.

## Checks worth rerunning after any change here

```sh
# the compiler is absent: only the stubs define these
arm-none-eabi-nm liblua.a | grep -E "luaY_parser|luaU_dump"

# the firmware's obligations have not grown
arm-none-eabi-nm --undefined-only liblua.a | grep fd_
```
