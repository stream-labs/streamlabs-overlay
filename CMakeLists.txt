cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

file(GLOB SOURCES "src/*.cpp" "include/*.h")

project(streamlabs_overlay_app)
add_executable(streamlabs_overlay_app ${SOURCES} ${WEBFORM_SOURCES})

if(WIN32)
   set_target_properties(streamlabs_overlay_app PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(streamlabs_overlay_app PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(streamlabs_overlay_app PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(streamlabs_overlay_app PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   set_target_properties(streamlabs_overlay_app PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(streamlabs_overlay_app PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(WIN32)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT streamlabs_overlay_app)
target_include_directories(streamlabs_overlay_app PRIVATE include webform)
install(TARGETS streamlabs_overlay_app DESTINATION bin)