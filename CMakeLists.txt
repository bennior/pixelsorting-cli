cmake_minimum_required(VERSION 3.22.1)

project(pixelsort)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(TARGET_OS "LINUX_OS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(TARGET_OS "WINDOWS_OS")
else()
  message(FATAL_ERROR "Operating System: ${CMAKE_SYSTEM_NAME} is not supported")
endif()

if(TARGET_OS STREQUAL "LINUX_OS")
  set(INCLUDE ${CMAKE_SOURCE_DIR}/include)
  file(GLOB LIB ${CMAKE_SOURCE_DIR}/lib/*.so*)
  file(GLOB RUNTIME ${LIB})
  set(BIN ${CMAKE_SOURCE_DIR}/bin)

  set(LIB_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/lib)
  set(BIN_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/bin)

  find_library(LIBM m)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

elseif(TARGET_OS STREQUAL "WINDOWS_OS")
  set(INCLUDE ${CMAKE_SOURCE_DIR}/include)
  file(GLOB LIB ${CMAKE_SOURCE_DIR}/win/lib/*.dll.a)
  file(GLOB RUNTIME ${CMAKE_SOURCE_DIR}/win/bin/*.dll)
  set(BIN ${CMAKE_SOURCE_DIR}/win/bin)

  set(LIB_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(BIN_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

  target_compile_definitions(pixelsort PUBLIC "WINDOWS_OS")

endif()

include_directories(${INCLUDE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})
set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb -O3 -ffast-math")
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
add_executable(pixelsort ${SOURCES})

target_link_libraries(pixelsort ${LIB} ${LIBM})

install(TARGETS pixelsort DESTINATION ${BIN_INSTALL_PREFIX})
install(PROGRAMS ${RUNTIME} DESTINATION ${LIB_INSTALL_PREFIX})
