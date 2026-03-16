# Helper to register a demo executable.
# Usage: celestia_add_demo(<target_name> <source_file>)
#
# Each demo:
#   - links against lua_static
#   - has the project root on its include path
#   - compiles with strict warnings

function(celestia_add_demo name source)
  add_executable(${name} ${source})
  target_include_directories(${name} PRIVATE "${CMAKE_SOURCE_DIR}")
  target_link_libraries(${name} PRIVATE celestia_core)
  if(MSVC)
    target_compile_options(${name} PRIVATE /W4 /WX)
  else()
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

# ---------- Register individual demos ----------
celestia_add_demo(demo_add    demos/add_demo.cpp)
celestia_add_demo(demo_greet  demos/greet_demo.cpp)
celestia_add_demo(demo_table  demos/table_demo.cpp)
celestia_add_demo(demo_script demos/script_demo.cpp)

# Copy the Lua script next to the built binary so it can be found at runtime
# regardless of the working directory.
add_custom_command(TARGET demo_script POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/demos/script_demo.lua"
    "$<TARGET_FILE_DIR:demo_script>/script_demo.lua"
  COMMENT "Copying script_demo.lua to output directory"
)

# Convenience target: build every demo at once
add_custom_target(all_demos
  DEPENDS demo_add demo_greet demo_table demo_script
  COMMENT "Build all demo targets"
)
