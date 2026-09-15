/*
** Flight Deck: stubs for the Lua source compiler.
**
** ldump.c is never built, so luaU_dump is defined here and reports failure:
** the device has no reason to write a binary chunk back out.
**
** lparser.c and lcode.c are built only when LUA_FD_WITH_PARSER is defined. In
** a build without them, luaY_parser is defined here and throws, so a chunk of
** source text is refused rather than reaching a compiler that is not present.
**
** Callers reach both symbols through lua_load and lua_dump, which run inside a
** protected call, so throwing here is reported as an ordinary Lua error rather
** than reaching the panic function.
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

#if !defined(LUA_FD_WITH_PARSER)

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

#endif /* !LUA_FD_WITH_PARSER */

int luaU_dump(lua_State *L, const Proto *f, lua_Writer w, void *data, int strip) {
    (void)L;
    (void)f;
    (void)w;
    (void)data;
    (void)strip;

    return 1; /* non-zero reports failure to lua_dump */
}
