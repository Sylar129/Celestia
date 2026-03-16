// Copyright Sylar129

#pragma once

extern "C" {
#include "lua/lua.h"
}

namespace celestia {

class State {
public:
  State();
  ~State();

private:
  lua_State *L;
};

} // namespace celestia
