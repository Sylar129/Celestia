// Copyright Sylar129

#include <cstdio>
#include <cstdlib>
#include <string>

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

// Demo: return a Lua table and read its fields from C++.
int main() {
  lua_State* L = luaL_newstate();
  if (L == nullptr) {
    std::fprintf(stderr, "Failed to create Lua state\n");
    return EXIT_FAILURE;
  }
  luaL_openlibs(L);

  const char kScript[] =
      "local planet = {\n"
      "  name   = \"Mars\",\n"
      "  radius = 3389.5,\n"
      "  moons  = 2,\n"
      "}\n"
      "return planet\n";

  if (luaL_dostring(L, kScript) != LUA_OK) {
    std::fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
    lua_close(L);
    return EXIT_FAILURE;
  }

  if (!lua_istable(L, -1)) {
    std::fprintf(stderr, "Expected table result\n");
    lua_close(L);
    return EXIT_FAILURE;
  }

  lua_getfield(L, -1, "name");
  const std::string name = lua_tostring(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, -1, "radius");
  const double radius = lua_tonumber(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, -1, "moons");
  const int moons = static_cast<int>(lua_tointeger(L, -1));
  lua_pop(L, 1);

  lua_pop(L, 1);  // pop table

  std::printf("Planet: %s, radius: %.1f km, moons: %d\n",
              name.c_str(), radius, moons);

  lua_close(L);
  return EXIT_SUCCESS;
}
