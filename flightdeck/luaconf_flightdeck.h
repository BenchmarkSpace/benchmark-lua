/*
** Flight Deck configuration for Lua.
**
** This header is force-included ahead of every Lua translation unit by the
** CMakeLists in this directory (-include). Every knob it sets is one that
** upstream guards with #if !defined(...), so nothing in the vendored Lua
** sources has to be edited to apply it. Keeping that property is the point of
** this file: `git merge upstream/master` on this fork touches only files Lua
** itself owns.
**
** The header is included before <stdio.h> and before lprefix.h, so it must not
** include any system header of its own. Lengths therefore cross the boundary
** as unsigned long rather than size_t.
*/

#ifndef LUACONF_FLIGHTDECK_H
#define LUACONF_FLIGHTDECK_H

/*
** Numeric model.
**
** LUA_32BITS is left at its upstream default of 0, so lua_Integer is 64 bits
** and lua_Number is a double. The SAMV71 FPU is double precision
** (-mfpu=fpv5-d16), so doubles are hardware backed, and 64-bit integers carry
** uid and other 64-bit attributes exactly. The cost is a 16-byte TValue rather
** than 8.
*/

/*
** Bound on nested reentrant calls into the VM: metamethods, pcall and native
** functions calling back into Lua. Lua-to-Lua calls reuse the interpreter's C
** frame and do not count against this, so the limit exists to bound C stack
** use rather than script call depth. Sized against the script task stack.
*/
#define LUAI_MAXCCALLS 60

/*
** String hashes are seeded from the device rather than from time() and the
** address of a local, which keeps <time.h> out of the build and makes the seed
** reproducible for a given boot.
*/
unsigned int fd_lua_seed(void);
#define luai_makeseed(L) (fd_lua_seed())

/*
** Console output.
**
** print() and the warning and panic paths reach the Flight Deck log component
** instead of stdout and stderr. lua_writestringerror is only ever called with
** a format string taking a single %s, so a two argument form is sufficient.
*/
#if defined(LUA_FD_HOST)

/* Host tools keep the upstream stdio behaviour. */

#else

void fd_lua_write(const char *s, unsigned long len);
void fd_lua_write_error(const char *fmt, const char *param);

#define lua_writestring(s, l) fd_lua_write((s), (unsigned long)(l))
#define lua_writeline() fd_lua_write("\n", 1)
#define lua_writestringerror(s, p) fd_lua_write_error((s), (const char *)(p))

#endif /* LUA_FD_HOST */

#endif /* LUACONF_FLIGHTDECK_H */
