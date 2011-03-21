
project("app")

cmake_minimum_required(VERSION 2.8)


#
# assigns sourcegroup from path
#
MACRO (sourceGroups sources)
	FOREACH (src ${${sources}})

		#delete source directory
		string(REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" last_dir ${src})
		#delete last slash and filename
		string(REGEX REPLACE "[\\\\/][^\\\\/]*$" "" last_dir ${last_dir})
		#delete first slash
		string(REGEX REPLACE "^[\\\\/]" "" last_dir ${last_dir})
		#replace forward with backslash
		string(REGEX REPLACE "/" "\\\\" last_dir ${last_dir})
		# assign sourcegroup
		source_group(${last_dir} FILES ${src})

	ENDFOREACH (src)
ENDMACRO (sourceGroups)

#
# create custom build command for glsl files
#
MACRO (generateCppFromGLSL sources outputs)
	FOREACH (src ${${sources}})
	
	# filter glsl files
	if(${src} MATCHES "\\.glsl$")

		# strip of path and just leave filename
		string(REGEX REPLACE "(.)*[\\\\/]" "" filename ${src})

		# assemble output filename
		set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${filename}.cpp )

		if( CMAKE_SYSTEM_NAME MATCHES Linux )
			set( pythoncommand "python2.6" )
		elseif( CMAKE_SYSTEM_NAME MATCHES Windows )
			set( pythoncommand "python.exe" )
		endif(CMAKE_SYSTEM_NAME MATCHES Windows)
		
		# add custom command
		add_custom_command( OUTPUT ${outfile}
							COMMAND ${pythoncommand} ${CMAKE_CURRENT_SOURCE_DIR}/src/base/buildtools/glsl2cpp.py ${src} > ${outfile}
							DEPENDS ${src} )

		# assign source group
		source_group(generated FILES ${outfile})
		
		# store output
		set( ${outputs} ${${outputs}} ${outfile} )
		
	endif(${src} MATCHES "\\.glsl$")

	ENDFOREACH (src)
ENDMACRO (generateCppFromGLSL)


#
# build app application
#

#specify app sources
file(GLOB_RECURSE app_sources src/*.cpp src/*.h src/*.glsl)
sourceGroups( app_sources )

#glsl build commands
generateCppFromGLSL(app_sources glsl_outputs)

# Create an executable
add_executable(app ${app_sources} ${glsl_outputs})

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
