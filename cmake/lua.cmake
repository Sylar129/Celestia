# Build Lua as an embedded static library using the upstream makefile layout.
# Exposes target: lua_static

include(CheckCCompilerFlag)

set(LUA_CORE_SOURCES
	"${CMAKE_SOURCE_DIR}/lua/lapi.c"
	"${CMAKE_SOURCE_DIR}/lua/lcode.c"
	"${CMAKE_SOURCE_DIR}/lua/lctype.c"
	"${CMAKE_SOURCE_DIR}/lua/ldebug.c"
	"${CMAKE_SOURCE_DIR}/lua/ldo.c"
	"${CMAKE_SOURCE_DIR}/lua/ldump.c"
	"${CMAKE_SOURCE_DIR}/lua/lfunc.c"
	"${CMAKE_SOURCE_DIR}/lua/lgc.c"
	"${CMAKE_SOURCE_DIR}/lua/llex.c"
	"${CMAKE_SOURCE_DIR}/lua/lmem.c"
	"${CMAKE_SOURCE_DIR}/lua/lobject.c"
	"${CMAKE_SOURCE_DIR}/lua/lopcodes.c"
	"${CMAKE_SOURCE_DIR}/lua/lparser.c"
	"${CMAKE_SOURCE_DIR}/lua/lstate.c"
	"${CMAKE_SOURCE_DIR}/lua/lstring.c"
	"${CMAKE_SOURCE_DIR}/lua/ltable.c"
	"${CMAKE_SOURCE_DIR}/lua/ltm.c"
	"${CMAKE_SOURCE_DIR}/lua/lundump.c"
	"${CMAKE_SOURCE_DIR}/lua/lvm.c"
	"${CMAKE_SOURCE_DIR}/lua/lzio.c"
	"${CMAKE_SOURCE_DIR}/lua/ltests.c"
)

set(LUA_AUX_SOURCES
	"${CMAKE_SOURCE_DIR}/lua/lauxlib.c"
)

set(LUA_LIB_SOURCES
	"${CMAKE_SOURCE_DIR}/lua/lbaselib.c"
	"${CMAKE_SOURCE_DIR}/lua/ldblib.c"
	"${CMAKE_SOURCE_DIR}/lua/liolib.c"
	"${CMAKE_SOURCE_DIR}/lua/lmathlib.c"
	"${CMAKE_SOURCE_DIR}/lua/loslib.c"
	"${CMAKE_SOURCE_DIR}/lua/ltablib.c"
	"${CMAKE_SOURCE_DIR}/lua/lstrlib.c"
	"${CMAKE_SOURCE_DIR}/lua/lutf8lib.c"
	"${CMAKE_SOURCE_DIR}/lua/loadlib.c"
	"${CMAKE_SOURCE_DIR}/lua/lcorolib.c"
	"${CMAKE_SOURCE_DIR}/lua/linit.c"
)

set(LUA_CWARNSCPP
	-Wfatal-errors
	-Wextra
	-Wshadow
	-Wundef
	-Wwrite-strings
	-Wredundant-decls
	-Wdisabled-optimization
	-Wdouble-promotion
	-Wmissing-declarations
	-Wconversion
)

set(LUA_CWARNSC
	-Wdeclaration-after-statement
	-Wmissing-prototypes
	-Wnested-externs
	-Wstrict-prototypes
	-Wc++-compat
	-Wold-style-definition
)

set(LUA_CWARNGCC)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	list(APPEND LUA_CWARNGCC
		-Wlogical-op
		-Wno-aggressive-loop-optimizations
	)
endif()

set(LUA_LOCAL
	${LUA_CWARNSCPP}
	${LUA_CWARNSC}
	${LUA_CWARNGCC}
)

set(LUA_CFLAGS
	-Wall
	-O2
	-fno-stack-protector
	-fno-common
	-march=native
	${LUA_LOCAL}
)

function(lua_append_supported_cflags target)
	foreach(flag IN LISTS ARGN)
		string(MAKE_C_IDENTIFIER "${flag}" flag_id)
		set(cache_var "LUA_HAS_${flag_id}")
		check_c_compiler_flag("${flag}" ${cache_var})
		if(${cache_var})
			target_compile_options(${target} PRIVATE "${flag}")
		endif()
	endforeach()
endfunction()

add_library(lua_static STATIC
	${LUA_CORE_SOURCES}
	${LUA_AUX_SOURCES}
	${LUA_LIB_SOURCES}
)
add_library(lua::static ALIAS lua_static)

set_target_properties(lua_static PROPERTIES
	OUTPUT_NAME lua
	C_STANDARD 99
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS NO
)

target_include_directories(lua_static PUBLIC "${CMAKE_SOURCE_DIR}/lua")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(lua_static PUBLIC LUA_USE_LINUX)
	target_link_libraries(lua_static PUBLIC m dl)
elseif(APPLE)
	target_compile_definitions(lua_static PUBLIC LUA_USE_MACOSX)
	target_link_libraries(lua_static PUBLIC m)
else()
	target_link_libraries(lua_static PUBLIC m)
endif()

lua_append_supported_cflags(lua_static ${LUA_CFLAGS})
