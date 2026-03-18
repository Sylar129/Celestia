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
}

State::~State() {
  if (L != nullptr) {
    lua_close(L);
  }
}

void State::OpenLibs() { luaL_openlibs(L); }

bool State::RunScript(const std::string &script) {
  if (luaL_dostring(L, script.c_str()) != LUA_OK) {
    CaptureErrorMessage();
    return false;
  }
  return true;
}

void State::RegisterFunction(const std::string& name, lua_CFunction func) {
  lua_register(L, name.c_str(), func);
}

template <> double State::GetLastValue<double>() const {
  if (!lua_isnumber(L, -1)) {
    throw std::runtime_error("Expected number at given index");
  }
  return lua_tonumber(L, -1);
}

void State::CaptureErrorMessage() {
  error_message_ = lua_tostring(L, -1);
  lua_pop(L, 1);
}

} // namespace celestia
