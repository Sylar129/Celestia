// Copyright Sylar129

#include "state.h"

#include <stdexcept>

extern "C" {
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

namespace celestia {
State::State() : L(luaL_newstate()) {
  if (L == nullptr) {
    throw std::runtime_error("Failed to create Lua state");
  }
  luaL_openlibs(L);
}

State::~State() {
  if (L != nullptr) {
    lua_close(L);
  }
}

} // namespace celestia
