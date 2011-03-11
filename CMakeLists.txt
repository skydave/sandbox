
project("app")

cmake_minimum_required(VERSION 2.8)


#
# get base sources
#
file(GLOB_RECURSE base_math_sources src/base/math/*.cpp src/base/math/*.h)
source_group( src\\base\\math FILES ${base_math_sources} )
file(GLOB_RECURSE base_ui_sources src/base/ui/*.cpp src/base/ui/*.h)
source_group( src\\base\\ui FILES ${base_ui_sources} )
file(GLOB_RECURSE base_gfx_sources src/base/gfx/*.cpp src/base/gfx/*.h)
source_group( src\\base\\gfx FILES ${base_gfx_sources} )
file(GLOB_RECURSE base_gltools_sources src/base/gltools/*.cpp src/base/gltools/*.h)
source_group( src\\base\\gltools FILES ${base_gltools_sources} )
file(GLOB_RECURSE base_util_sources src/base/util/*.cpp src/base/util/*.h)
source_group( src\\base\\util FILES ${base_util_sources} )

#
# build app application
#

#specify app sources
file(GLOB app_sources src/*.cpp)
source_group( src FILES ${app_sources} )

# Create an executable
add_executable(app ${app_sources} ${base_math_sources} ${base_ui_sources} ${base_gfx_sources} ${base_gltools_sources} ${base_util_sources})

# add base to include directories
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/base )

#
# set link dependecies
#
if( CMAKE_SYSTEM_NAME MATCHES Linux )
	find_package(X11)

	if(NOT X11_FOUND)
		message(FATAL_ERROR "Failed to find X11 which is required to build under linux")
	endif(NOT X11_FOUND)


	target_link_libraries(app ${X11_LIBRARIES})

endif( CMAKE_SYSTEM_NAME MATCHES Linux )

INCLUDE(FindOpenGL)
target_link_libraries(app ${OPENGL_gl_LIBRARY})
