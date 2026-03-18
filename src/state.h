// Copyright Sylar129

#pragma once

#include <string>

extern "C" {
#include "lua/lua.h"
}

namespace celestia {

// ---------------------------------------------------------------------------
// LuaValue
//
// Discriminated union for Lua's primitive types:
//   std::monostate -> nil
//   bool           -> boolean
//   double         -> number
//   std::string    -> string
// ---------------------------------------------------------------------------
using LuaValue = std::variant<std::monostate, bool, double, std::string>;

// Returns true if |v| holds nil.
inline bool IsNil(const LuaValue& v) {
  return std::holds_alternative<std::monostate>(v);
}

// Returns the T inside |v|, or |fallback| when the active alternative is not T.
template <typename T>
inline T GetValue(const LuaValue& v, const T& fallback = {}) {
  const auto* ptr = std::get_if<T>(&v);
  return ptr ? *ptr : fallback;
}

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

  void RegisterFunction(const std::string& name, lua_CFunction func);

  [[nodiscard]] std::string GetErrorMessage() const { return error_message_; }

 private:
  void CaptureErrorMessage();
  std::string error_message_;
  lua_State *L;
};

} // namespace celestia
