# Build the celestia core library from src/.
# Exposes target: celestia_core

file(GLOB _core_sources "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_library(celestia_core STATIC ${_core_sources})
target_include_directories(celestia_core
  PUBLIC  "${CMAKE_SOURCE_DIR}/src"
  PRIVATE "${CMAKE_SOURCE_DIR}"
)
target_link_libraries(celestia_core PUBLIC lua_static)
if(MSVC)
  target_compile_options(celestia_core PRIVATE /W4 /WX)
else()
  target_compile_options(celestia_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
