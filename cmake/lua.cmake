# Build Lua 5.x from source as a static library.
# Exposes target: lua_static

file(GLOB _lua_sources "${CMAKE_SOURCE_DIR}/lua/*.c")
list(FILTER _lua_sources EXCLUDE REGEX "lua\\.c$")      # stand-alone interpreter
list(FILTER _lua_sources EXCLUDE REGEX "onelua\\.c$")   # amalgamation driver
list(FILTER _lua_sources EXCLUDE REGEX "ltests\\.c$")   # internal test helpers

add_library(lua_static STATIC ${_lua_sources})
target_include_directories(lua_static PUBLIC "${CMAKE_SOURCE_DIR}/lua")
