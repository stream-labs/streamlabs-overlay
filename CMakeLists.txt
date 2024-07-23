cmake_minimum_required(VERSION 3.11)
project(game_overlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(NODEJS_URL "https://artifacts.electronjs.org/headers/dist" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v29.4.3" CACHE STRING "Node.JS Version")

include(NodeJS)

nodejs_init()

set(OVERLAY_SOURCES
	src/main.cpp
	src/module.cpp
	src/overlay_logging.cpp
	src/overlay_paint_frame_js.cpp
	src/overlay_paint_frame.cpp
	src/sl_overlay_api.cpp
	src/sl_overlay_window.cpp
	src/sl_overlays_settings.cpp
	src/sl_overlays.cpp
	src/stdafx.cpp
	src/user_input_callback.cpp )

SET(PROJECT_INCLUDE_PATHS ${NODEJS_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include/" )

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

list(APPEND PROJECT_INCLUDE_PATHS ${NODE_ADDON_API_DIR})

add_definitions(-DNAPI_VERSION=7)

add_nodejs_module(${PROJECT_NAME} ${OVERLAY_SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} d2d1.lib dwrite.lib Dwmapi.lib)

target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_PATHS})
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_NODE_EXTENSION)

set(CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
	CMAKE_C_FLAGS_RELWITHDEBINFO
	)
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)

include(FetchContent)

# Compare current linked libs with prev
FetchContent_Declare(
	deps_checker
	URL "https://raw.githubusercontent.com/stream-labs/obs-studio-node/staging/dependency_checker/check_dependencies.cmd"
	DOWNLOAD_NO_EXTRACT true
)
FetchContent_MakeAvailable(deps_checker)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
	COMMAND ${deps_checker_SOURCE_DIR}/check_dependencies.cmd $<TARGET_FILE:game_overlay> ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} $<CONFIG>
)


SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Print all linked libs
get_target_property(LIST_OF_LIBRARIES ${PROJECT_NAME} LINK_LIBRARIES)
message(STATUS "List of libraries to link with: " "${LIST_OF_LIBRARIES}")

# Install
install(FILES $<TARGET_PDB_FILE:game_overlay> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)

install(FILES $<TARGET_FILE:game_overlay> DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	FILES
		${game_overlay_SOURCE_DIR}/npm/package.json
		${game_overlay_SOURCE_DIR}/npm/index.js
		${game_overlay_SOURCE_DIR}/npm/typings.d.ts
	DESTINATION
		${CMAKE_INSTALL_PREFIX})
