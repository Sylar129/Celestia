// Copyright Sylar129

#include <cstdio>
#include <cstdlib>

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

namespace {

// C function exposed to Lua as the global 'greet(name)'.
int LuaGreet(lua_State* L) {
  const char* name = luaL_optstring(L, 1, "World");
  std::printf("Hello, %s!\n", name);
  return 0;  // no Lua return values
}

}  // namespace

// Demo: register a C++ function into the Lua global table and call it.
int main() {
  lua_State* L = luaL_newstate();
  if (L == nullptr) {
    std::fprintf(stderr, "Failed to create Lua state\n");
    return EXIT_FAILURE;
  }
  luaL_openlibs(L);

  lua_pushcfunction(L, LuaGreet);
  lua_setglobal(L, "greet");

  const char kScript[] = "greet(\"Celestia\")\n";
  if (luaL_dostring(L, kScript) != LUA_OK) {
    std::fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
    lua_close(L);
    return EXIT_FAILURE;
  }

  lua_close(L);
  return EXIT_SUCCESS;
}
