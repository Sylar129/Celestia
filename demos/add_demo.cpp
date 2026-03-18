// Copyright Sylar129

#include <cstdio>
#include <cstdlib>

#include "state.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

const char kScript[] =
    "function add(a, b)\n"
    "  return a + b\n"
    "end\n"
    "return add(10, 32)\n";

// Demo: define and call a Lua function from C++, then read the return value.
int main() {
  // lua_State* L = luaL_newstate();
  // if (L == nullptr) {
  //   std::fprintf(stderr, "Failed to create Lua state\n");
  //   return EXIT_FAILURE;
  // }
  // luaL_openlibs(L);

  // if (luaL_dostring(L, kScript) != LUA_OK) {
  //   std::fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
  //   lua_close(L);
  //   return EXIT_FAILURE;
  // }

  // if (!lua_isnumber(L, -1)) {
  //   std::fprintf(stderr, "Expected number result\n");
  //   lua_close(L);
  //   return EXIT_FAILURE;
  // }

  // const double result = lua_tonumber(L, -1);
  // lua_pop(L, 1);
  // std::printf("add(10, 32) = %.0f\n", result);

  // lua_close(L);

  celestia::State state;
  state.OpenLibs();
  if (!state.RunScript(kScript)) {
    std::printf("Lua error: %s\n", state.GetErrorMessage().c_str());
    return EXIT_FAILURE;
  }
  double v = state.GetLastValue<double>();
  std::printf("C++: add(10, 32) = %.0f\n", v);

  double vv = state.GetLastValue<double>();
  std::printf("C++: add(10, 32) again = %.0f\n", vv);

  return EXIT_SUCCESS;
}
