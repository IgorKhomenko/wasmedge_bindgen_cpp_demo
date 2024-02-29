cmake_minimum_required(VERSION 3.8)

project(z_validator)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)
# Set C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -frtti -fthreadsafe-statics -w")

# Set C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

include_directories(
    /usr/include
    /usr/local/lib
    /usr/local/include
)

# https://stackoverflow.com/questions/47690827/cmake-library-not-found
# When your library isn't in a standard path liking /usr/lib, you should use link_directories() in your CMakeLists.txt to specify a non-standard library path which contains your library. Notice that you MUST put your link_directories() ahead of add_executable() as the following shows:
if(APPLE)
  link_directories(
    /opt/homebrew/lib
  )
endif()

# Specify the source files
set(SOURCES
    main.cpp
)

# Set compile options
add_compile_options(
    -Wswitch
    -Wno-deprecated-declarations
    -Wempty-body
    #-Wconversion
    -Wreturn-type
    -Wparentheses
    -Wno-format
    -Wuninitialized
    -Wunreachable-code
    #-Wunused-function
    #-Wno-unused-function
    #-Wunused-value
    #-Wunused-variable
    #-std=c++17
    -Wall
    -fno-strict-aliasing
    -g2
    -gdwarf-2
    -O0
    #-frtti
    -fno-omit-frame-pointer
    #-std=c11
    -fexceptions
)

# Create the executable
add_executable(app-externref ${SOURCES})

# Link necessary libraries
target_link_libraries(app-externref
    wasmedge
)
