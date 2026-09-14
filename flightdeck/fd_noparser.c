/*
** Flight Deck: stubs for the Lua source compiler.
**
** The target build omits lparser.c, lcode.c and ldump.c: scripts are compiled
** on a development machine and reach the device as binary chunks. Two symbols
** in the remaining sources still reference the compiler, and both are defined
** here so that no vendored file has to be edited.
**
**   luaY_parser  ldo.c calls it from f_parser when a chunk does not begin with
**                LUA_SIGNATURE, that is, when the chunk is source text.
**   luaU_dump    lapi.c calls it from lua_dump.
**
** Callers reach these through lua_load and lua_dump, both of which run inside
** a protected call, so throwing here is reported to the caller as an ordinary
** Lua error rather than reaching the panic function.
**
** The scripts component also passes mode "b" to lua_load, which makes ldo.c
** reject a text chunk in checkmode before it reaches luaY_parser. This stub is
** the backstop for any path that does not set a mode.
*/

#define fd_noparser_c
#define LUA_CORE

#include "lprefix.h"

#include "lua.h"

#include "ldo.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstate.h"
#include "lundump.h"
#include "lzio.h"

LClosure *luaY_parser(
    lua_State *L, ZIO *z, Mbuffer *buff, Dyndata *dyd, const char *name, int firstchar
) {
    (void)z;
    (void)buff;
    (void)dyd;
    (void)name;
    (void)firstchar;

    luaD_throw(L, LUA_ERRSYNTAX);

    return NULL; /* not reached */
}

int luaU_dump(lua_State *L, const Proto *f, lua_Writer w, void *data, int strip) {
    (void)L;
    (void)f;
    (void)w;
    (void)data;
    (void)strip;

    return 1; /* non-zero reports failure to lua_dump */
}
