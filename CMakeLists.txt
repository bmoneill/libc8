# See https://honeytreelabs.com/posts/cmake-unity-integration

cmake_minimum_required(VERSION 3.31.6)
project("libc8" C)
include(CTest)

add_subdirectory(docs/man)

# defaults
option(SDL2 "Enable SDL2 support" ON)
option(TEST "Build tests" OFF)


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
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math")

function(Enable_Tests)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -fprofile-arcs -ftest-coverage -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
    include(CodeCoverage)
endfunction()

function(SDL2_Required)
    message("Note: SDL2 is required here - make sure this is run with -DSDL2=ON")
endfunction()

function(Build_Library)
    add_subdirectory(src)
    include_directories(src)
endfunction()

function(Build_Tests)
    add_subdirectory(external)
    add_subdirectory(test)
endfunction()

if(TEST)
    Enable_Tests()
    Build_Library()
    Build_Tests()
elseif(TARGET_GROUP STREQUAL lib)
    SDL2_Required()
    Build_Library()
else()
    SDL2_Required()
    Build_Library()
    add_subdirectory(tools)
endif()
