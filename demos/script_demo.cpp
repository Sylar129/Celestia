// Copyright Sylar129

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

// Demo: load and execute a .lua script file, then read the returned table.
//
// Usage: demo_script [path/to/script.lua]
// Defaults to "demos/script_demo.lua" relative to the working directory.

int main(int argc, char* argv[]) {
  const char *kDefaultScript = "script_demo.lua";
  const char* script_path = (argc >= 2) ? argv[1] : kDefaultScript;

  lua_State* L = luaL_newstate();
  if (L == nullptr) {
    std::fprintf(stderr, "Failed to create Lua state\n");
    return EXIT_FAILURE;
  }
  luaL_openlibs(L);

  // luaL_dofile compiles and runs the file; any returned values are pushed.
  if (luaL_dofile(L, script_path) != LUA_OK) {
    std::fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
    lua_close(L);
    return EXIT_FAILURE;
  }

  // Read the table returned by the script (if any).
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "author");
    const std::string author =
        lua_isstring(L, -1) ? lua_tostring(L, -1) : "(unknown)";
    lua_pop(L, 1);

    lua_getfield(L, -1, "version");
    const std::string version =
        lua_isstring(L, -1) ? lua_tostring(L, -1) : "(unknown)";
    lua_pop(L, 1);

    lua_getfield(L, -1, "result");
    const lua_Integer result =
        lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 0;
    lua_pop(L, 1);

    lua_pop(L, 1);  // pop table

    std::printf("Script metadata — author: %s, version: %s, result: %lld\n",
                author.c_str(), version.c_str(),
                static_cast<long long>(result));
  }

  lua_close(L);
  return EXIT_SUCCESS;
}
