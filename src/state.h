// Copyright Sylar129

#pragma once

#include <string>

extern "C" {
#include "lua/lua.h"
}

namespace celestia {

class State {
public:
  State();
  ~State();

  void OpenLibs();

  bool RunScript(const std::string &script);

  template <typename T>
  T GetLastValue() const;

  template <>
  double GetLastValue<double>() const;

  [[nodiscard]] std::string GetErrorMessage() const { return error_message_; }

 private:
  void CaptureErrorMessage();
  std::string error_message_;
  lua_State *L;
};

} // namespace celestia
