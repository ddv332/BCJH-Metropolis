cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")
# set(CMAKE_BUILD_TYPE "Debug")

set(PJNAME bcjh)
project(${PJNAME})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 7.1
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
        message(FATAL_ERROR "GCC version must be at least 10 to compile c++20!" ${CMAKE_CXX_COMPILER_VERSION})
    endif()
endif()
message(STATUS "operation system is ${CMAKE_SYSTEM_NAME}")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:gbk>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W3>")
    
    message(STATUS "Now is windows")
elseif(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    # Doesn't work on Windows: I don't know how to specify the charset.
    # If I use utf8, running on windows would be cause error C2001 etc;
    # If I use utf8 with BOM, there would be extra # before some file and invoke error.
    # add_compile_options("-pthread")
    # add_compile_options("-sNO_DISABLE_EXCEPTION_CATCHING")
    message(STATUS "Now is Emscripten")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    add_definitions(-DEMSCRIPTEN)
    # execute_process(COMMAND bash assets/removeSpaces.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Spaces and newlines in data.min.json have been removed.")
else()
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread -Wall -Wextra -Wno-reorder -Wno-unused-parameter")
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src src)


file(GLOB src_list_cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
                       ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
                      )
set(src_EMSCRIPTEN  ${src_list_cpp})
list(REMOVE_ITEM src_EMSCRIPTEN ${CMAKE_CURRENT_SOURCE_DIR}/src/bcjh_js_cpp.cpp)
list(REMOVE_ITEM src_EMSCRIPTEN ${CMAKE_CURRENT_SOURCE_DIR}/src/bcjh_win.cpp)

set(src_WINDOWS  ${src_list_cpp})
list(REMOVE_ITEM src_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/src/bcjh_js_cpp.cpp)
list(REMOVE_ITEM src_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/src/bcjh_js.cpp)

set(src_LINUX  ${src_list_cpp})
list(REMOVE_ITEM src_LINUX ${CMAKE_CURRENT_SOURCE_DIR}/src/bcjh_win.cpp)

	
message(STATUS "target name is ${PJNAME}")
message(STATUS "src_list_cpp is ${src_list_cpp}")

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    # set multithread make
    set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j$(nproc)")
    
    foreach(i 2 1 3)
        set(target_name ${PJNAME}_${i})
        add_executable(${target_name} ${src_EMSCRIPTEN})
        target_compile_definitions(${target_name} PRIVATE -DNUM_GUESTS=${i})
        set_target_properties(${target_name} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/data.min.json)
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/data/data.min.json@./data.min.json -lembind -sALLOW_MEMORY_GROWTH -sEXPORTED_RUNTIME_METHODS=run")
        # Add -O0 and -fexceptions to LINK_FLAGS when -DCMAKE_BUILD_TYPE=Debug; ow. -O3
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS_DEBUG "-O0 -fexceptions")
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS_RELEASE "-O3")
    endforeach()
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(${PJNAME} ${src_WINDOWS})
else()
    add_executable(${PJNAME} ${src_LINUX})
    set(target_WINDOWS ${PJNAME}_win)
    add_executable(${target_WINDOWS} ${src_WINDOWS})
    target_compile_definitions(${target_WINDOWS} PRIVATE -D_WIN32)
endif()


