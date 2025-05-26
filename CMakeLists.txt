cmake_minimum_required(VERSION 3.22.1)

project(pixelsort)

# Set output directory for the executable
set(BIN ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})

# Source and include directories
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_SOURCE_DIR}/include)
set(LIB ${CMAKE_SOURCE_DIR}/lib)

# Add this line, to enable compile command export
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find all source files in src/
file(GLOB SOURCES ${SRC}/*.c)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -ggdb -O3")

# Add include directories
include_directories(${INCLUDE})

# Create the executable
add_executable(pixelsort ${SOURCES})

# Link FFmpeg libraries
add_library(libraries SHARED IMPORTED)
set_target_properties(libraries PROPERTIES IMPORTED_LOCATION ${LIB}/*.so)
target_link_libraries(pixelsort libraries)
