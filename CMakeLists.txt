cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

file(GLOB SOURCES "src/*.cpp" "include/*.h")
file(GLOB WEBFORM_SOURCES "webform/*.cpp" "webform/*.h")

configure_file( "help.html" ${CMAKE_CURRENT_BINARY_DIR}/help.html COPYONLY)
 
project(streamlabs_overlay)
add_executable(streamlabs_overlay ${SOURCES} ${WEBFORM_SOURCES})

if(WIN32)
   set_target_properties(streamlabs_overlay PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(streamlabs_overlay PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(streamlabs_overlay PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(streamlabs_overlay PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   set_target_properties(streamlabs_overlay PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(streamlabs_overlay PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(WIN32)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT streamlabs_overlay)
target_include_directories(streamlabs_overlay PRIVATE include webform)
install(TARGETS streamlabs_overlay DESTINATION bin)