# See https://honeytreelabs.com/posts/cmake-unity-integration

cmake_minimum_required(VERSION 3.31.6)
project("libc8" C)
include(CTest)

add_subdirectory(docs/man)

# defaults
set(SDL2 OFF)


# Store git commit hash in GIT_COMMIT_HASH
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Version
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DC8_VERSION='\"${GIT_COMMIT_HASH}\"'")

# C Standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_FILE_OFFSET_BITS=64")

# Warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-missing-field-initializers")

# Optimization
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math")


function(SDL2_Required)
    if(SDL2 EQUAL OFF)
        message(STATUS "SDL2 required for tools target group, setting SDL2 to ON")
        set(SDL2 ON CACHE INTERNAL "")
    endif()
endfunction()

function(Enable_Tests)
    SDL2_Required()
    set(TEST ON CACHE INTERNAL "")
    if(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -fprofile-arcs -ftest-coverage -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
        include(CodeCoverage)
    endif()
endfunction()

function(Build_Library)
    add_subdirectory(src)
    include_directories(src)
endfunction()

function(Build_Tests)
    add_subdirectory(external)
    add_subdirectory(test)
endfunction()

if(TARGET_GROUP STREQUAL all)
    SDL2_Required()
    Enable_Tests()
    Build_Library()
    Build_Tests()
    add_subdirectory(tools)
elseif(TARGET_GROUP STREQUAL test)
    Enable_Tests()
    Build_Library()
    Build_Tests()
elseif(TARGET_GROUP STREQUAL tools)
    SDL2_Required()
    Build_Library()
    add_subdirectory(tools)
elseif(TARGET_GROUP STREQUAL lib)
    SDL2_Required()
    Build_Library()
else()
    Build_Library()
    add_subdirectory(tools)
endif()
