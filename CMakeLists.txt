#-DCMAKE_BUILD_TYPE=Debug

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
# create moc files for each header which contains string "Q_OBJECT"
# and ui header files for each .ui file in sources
#
MACRO (generateQtOutputs sources moc_outputs ui_outputs)
	#
	# moc output
	#
	set( moc_headers )
	#for each source
	FOREACH (src ${${sources}})
		# filter header files
		if(${src} MATCHES "\\.h$")
			# read header file
			file(READ ${src} tmp)
			# check if it contains the string "Q_OBJECT"
			string(REGEX MATCH ".*Q_OBJECT.*" contains_q_object_string "${tmp}")
			if( contains_q_object_string )
				set( moc_headers ${moc_headers} ${src} )
			endif( contains_q_object_string )
		endif(${src} MATCHES "\\.h$")
	ENDFOREACH (src)

	QT4_WRAP_CPP( ${moc_outputs} ${moc_headers})
	source_group(generated FILES ${${moc_outputs}})

	#
	# ui output
	#
	set( ui_files )
	#for each source
	FOREACH (src ${${sources}})
		# filter ui files
		if(${src} MATCHES "\\.ui$")
			set( ui_files ${ui_files} ${src} )
		endif(${src} MATCHES "\\.ui$")
	ENDFOREACH (src)

	QT4_WRAP_UI( ${ui_outputs} ${ui_files})
	source_group(generated FILES ${${ui_outputs}})
ENDMACRO (generateQtOutputs)


add_definitions(-DSRC_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\")


# on windows
if( CMAKE_SYSTEM_NAME MATCHES Windows )
	add_definitions(-DNOMINMAX) # windows defines min and max in winbase.h which clashes with stl
	add_definitions(-DPA_USE_DS) # portaudio to use directx sound on windows

	# static runtime (http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F)
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif( CMAKE_SYSTEM_NAME MATCHES Windows )

# on linux
if( CMAKE_SYSTEM_NAME MATCHES Linux )
	add_definitions(-DPA_USE_ALSA) # portaudio to use alsa on linux
endif( CMAKE_SYSTEM_NAME MATCHES Linux )





#
# QT
#
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


# findFBX =========
if( CMAKE_SYSTEM_NAME MATCHES Windows )
	get_filename_component( FBXSDKPATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Autodesk FBX SDK 2012.2;Install_Dir]" PATH)
	set( FBX_LIBRARIES "optimized;${FBXSDKPATH}/2012.2/lib/vs2010/x64/fbxsdk-2012.2-mt.lib;debug;${FBXSDKPATH}/2012.2/lib/vs2010/x64/fbxsdk-2012.2-mtd.lib" )
	set( FBX_INCLUDES "${FBXSDKPATH}/2012.2/include" )
endif( CMAKE_SYSTEM_NAME MATCHES Windows )
# on linux
if( CMAKE_SYSTEM_NAME MATCHES Linux )
	set( FBXSDKPATH $ENV{FBXSDKPATH} )
	set( FBX_LIBRARIES "optimized;${FBXSDKPATH}/lib/gcc4/x64/libfbxsdk-2012.2-static.a;debug;${FBXSDKPATH}/lib/gcc4/x64/libfbxsdk-2012.2-staticd.a" )
	set( FBX_INCLUDES "${FBXSDKPATH}/include" )
endif( CMAKE_SYSTEM_NAME MATCHES Linux )


include_directories( ${FBX_INCLUDES} )

#
# build app application
#

#specify app sources
file(GLOB_RECURSE app_sources src/*.c src/*.cpp src/*.h src/*.hpp src/*.glsl src/*.ui)
sourceGroups( app_sources )


#qt moc and ui
generateQtOutputs( app_sources moc_outputs ui_outputs)

# Create an executable
add_executable(app ${app_sources} ${moc_outputs} ${ui_outputs})

# add base to include directories
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/base )

# we compile some third party code with base which we want to include conveniently
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/base/3rdparty )

# qt ui headers are generated in build dir
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

#
# set link dependecies
#

# on linux
if( CMAKE_SYSTEM_NAME MATCHES Linux )
	# X11
	find_package(X11)

	if(NOT X11_FOUND)
		message(FATAL_ERROR "Failed to find X11 which is required to build under linux")
	endif(NOT X11_FOUND)

	target_link_libraries(app ${X11_LIBRARIES})

	# alsa
	find_package(ALSA)

	if(NOT ALSA_FOUND)
		message(FATAL_ERROR "Failed to find ALSA which is required to build under linux")
	endif(NOT ALSA_FOUND)

	target_link_libraries(app ${ALSA_LIBRARIES})

endif( CMAKE_SYSTEM_NAME MATCHES Linux )

# OpenGL
INCLUDE(FindOpenGL)
target_link_libraries(app ${OPENGL_gl_LIBRARY})

# Qt
target_link_libraries(app ${QT_LIBRARIES})

# FBX
target_link_libraries(app ${FBX_LIBRARIES})
