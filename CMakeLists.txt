cmake_minimum_required(VERSION 3.11)
project(game-overlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v9.3.3" CACHE STRING "Node.JS Version")

include(NodeJS)
include(ExternalProject)

nodejs_init()

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=4)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR} "src/*.cpp")

add_nodejs_module(game-overlay ${sources})

target_link_libraries(game-overlay ${NODEJS_LIBRARIES})
target_link_libraries(game-overlay d2d1.lib dwrite.lib Dwmapi.lib)

target_include_directories(game-overlay PUBLIC ${NODEJS_INCLUDE_DIRS} ${NODE_ADDON_API_DIR} "include/")
 
set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17 )

target_compile_definitions(game-overlay PRIVATE -DUNICODE -D_UNICODE)

IF(WIN32)
	install(FILES $<TARGET_PDB_FILE:game-overlay> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
ENDIF()

install(FILES $<TARGET_FILE:game-overlay> DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	FILES
		${game-overlay_SOURCE_DIR}/npm/package.json
		${game-overlay_SOURCE_DIR}/npm/index.js
		${game-overlay_SOURCE_DIR}/npm/typings.d.ts
	DESTINATION
		${CMAKE_INSTALL_PREFIX})
