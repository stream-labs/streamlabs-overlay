cmake_minimum_required(VERSION 3.11)
project(game_overlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v13.4.0" CACHE STRING "Node.JS Version")

include(NodeJS)

nodejs_init()

SET(PROJECT_INCLUDE_PATHS ${NODEJS_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include/" )

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

list(APPEND PROJECT_INCLUDE_PATHS ${NODE_ADDON_API_DIR})
# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=7)

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
add_nodejs_module(${PROJECT_NAME} ${OVERLAY_SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} d2d1.lib dwrite.lib Dwmapi.lib)

target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_PATHS})
set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17 )
target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE BUILDING_NODE_EXTENSION -D_CRT_SECURE_NO_WARNINGS)

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

SET_TARGET_PROPERTIES(
	${PROJECT_NAME}
	PROPERTIES
	PREFIX ""
	SUFFIX ".node"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
	COMMAND dumpbin /DEPENDENTS $<TARGET_FILE:game_overlay> > ${CMAKE_CURRENT_BINARY_DIR}/current_dependents.txt
	COMMAND powershell -command "Get-Content ${CMAKE_CURRENT_BINARY_DIR}/current_dependents.txt | Select-String -Pattern Summary -Context 999,0 > ${CMAKE_CURRENT_BINARY_DIR}/current_dependents_cut.txt"
	COMMAND powershell -command "compare-object (get-content  ${CMAKE_CURRENT_BINARY_DIR}/current_dependents_cut.txt) (get-content  ${CMAKE_SOURCE_DIR}/cmake/approved_dependents.txt)"
	COMMAND powershell -command "if( compare-object (get-content  ${CMAKE_CURRENT_BINARY_DIR}/current_dependents_cut.txt) (get-content  ${CMAKE_SOURCE_DIR}/cmake/approved_dependents.txt) ) { Write-Error 'Alarm: Module dependencies changed. Please review changes.' }"
)

IF(WIN32)
	install(FILES $<TARGET_PDB_FILE:game_overlay> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
ENDIF()

install(FILES $<TARGET_FILE:game_overlay> DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	FILES
		${game_overlay_SOURCE_DIR}/npm/package.json
		${game_overlay_SOURCE_DIR}/npm/index.js
		${game_overlay_SOURCE_DIR}/npm/typings.d.ts
	DESTINATION
		${CMAKE_INSTALL_PREFIX})
