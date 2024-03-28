cmake_minimum_required(VERSION 3.20)

add_compile_definitions(
    IS_UNIX="1"
)

project(genpathmaps C)

file (GLOB_RECURSE genpathmaps_SOURCES CONFIGURE_DEPENDS "*.c")
file (GLOB_RECURSE genpathmaps_HEADERS CONFIGURE_DEPENDS "*.h")

file(GLOB_RECURSE REMOVE_CMAKE "CMakeFiles/*")
list(REMOVE_ITEM genpathmaps_SOURCES ${REMOVE_CMAKE})

set (genpathmaps_INCLUDE_DIRS "")
foreach (_headerFile ${genpathmaps_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND genpathmaps_INCLUDE_DIRS ${_dir})
endforeach()
list (REMOVE_DUPLICATES genpathmaps_INCLUDE_DIRS)

add_executable(genpathmaps ${genpathmaps_SOURCES})
target_include_directories(genpathmaps PRIVATE ${genpathmaps_INCLUDE_DIRS})
