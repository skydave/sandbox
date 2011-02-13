project("fem")

cmake_minimum_required(VERSION 2.6)



#
# get base sources
#
file(GLOB_RECURSE base_math_sources src/base/math/*.cpp src/base/math/*.h)
source_group( base\\math FILES ${base_math_sources} )
file(GLOB_RECURSE base_ui_sources src/base/ui/*.cpp src/base/ui/*.h)
source_group( base\\ui FILES ${base_ui_sources} )

#
# build fem application
#

#specify app sources
file(GLOB fem_sources src/*.cpp)
source_group( fem FILES ${fem_sources} )

# Create an executable
add_executable(fem ${fem_sources} ${base_math_sources} ${base_ui_sources})

# add base to include directories
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/base )