cmake_minimum_required(VERSION 3.16)

project(lua
    VERSION 5.4.9
    LANGUAGES C
)

#
# Flight Deck build of Lua.
#
# Two things distinguish it from an ordinary Lua build.
#
# The source compiler is optional. By default lparser.c, lcode.c and ldump.c
# are not built and flightdeck/fd_noparser.c supplies the symbols the remaining
# sources reference: scripts are compiled on a development machine and reach
# the device as binary chunks, which keeps the compiler out of the image and
# out of the arena.
#
# LUA_FD_PARSER=ON builds lparser.c and lcode.c so the device accepts source
# text. That costs flash and, more tightly, arena: a parse holds the whole
# function under construction alongside the interpreter.
#
# Only the libraries a script is permitted to use are built. io, os, package,
# debug, coroutine and utf8 are absent, so they cannot be opened by accident.
# linit.c is absent too: the scripts component opens libraries itself, which
# keeps the permitted set in one place.
#
# Nothing in the vendored Lua sources is edited. Configuration is applied
# through flightdeck/luaconf_flightdeck.h, force-included ahead of every
# translation unit.
#

set(LUA_CORE_SOURCES
    lapi.c
    lctype.c
    ldebug.c
    ldo.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lstate.c
    lstring.c
    ltable.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c
)

set(LUA_LIB_SOURCES
    lauxlib.c
    lbaselib.c
    lmathlib.c
    lstrlib.c
    ltablib.c
)

option(LUA_FD_PARSER "Build the Lua source compiler into the target library" OFF)

set(LUA_FLIGHTDECK_SOURCES
    flightdeck/fd_noparser.c
)

if(LUA_FD_PARSER)
    list(APPEND LUA_CORE_SOURCES
        lparser.c
        lcode.c
    )
endif()

add_library(${PROJECT_NAME} STATIC
    ${LUA_CORE_SOURCES}
    ${LUA_LIB_SOURCES}
    ${LUA_FLIGHTDECK_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# The maths library. The interpreter reaches it for the ^ and % operators as
# well as for the math library, so it is a property of Lua rather than of any
# one application, and is carried here for anything that links Lua.
target_link_libraries(${PROJECT_NAME} PUBLIC
    m
)

# Don't use target_compile_features (toolchain skip breaks feature detection).
# Force C99 on the target instead.
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

# The configuration header must be visible to every translation unit before
# luaconf.h and lauxlib.h are reached, so that the #if !defined guards upstream
# already has pick it up. PUBLIC because anything including lua.h needs the
# same view of the configuration as the library was built with.
target_compile_options(${PROJECT_NAME} PUBLIC
    -include ${CMAKE_CURRENT_LIST_DIR}/flightdeck/luaconf_flightdeck.h
)

# PUBLIC so that a caller choosing the load mode sees the same view of the
# library as the library was built with.
if(LUA_FD_PARSER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LUA_FD_WITH_PARSER)
endif()

# Optimise this target for size and let the linker discard what the permitted
# library set does not reach. --gc-sections is a link option and belongs to the
# application.
target_compile_options(${PROJECT_NAME} PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

# Vendored upstream code: warnings here are not actionable without diverging
# from upstream, and the project builds with -Wall -Wformat=2 globally.
target_compile_options(${PROJECT_NAME} PRIVATE
    -w
)
