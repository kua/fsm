project(fsm)
cmake_minimum_required(VERSION 3.0)

add_definitions ("-Wall")
add_definitions ("-Werror")
add_definitions ("-fno-strict-aliasing")
add_definitions ("-pedantic")
add_definitions ("-std=gnu++11")

########### gmock find stub

find_path(GMOCK_INCLUDE_DIR gmock/gmock.h)
mark_as_advanced(GMOCK_INCLUDE_DIR)

find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMock DEFAULT_MSG GMOCK_LIBRARY GMOCK_INCLUDE_DIR GMOCK_MAIN_LIBRARY)

if(GMOCK_FOUND)
    set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR})
    set(GMOCK_LIBRARIES ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY})
endif()

##########

find_package(GTest REQUIRED)
#find_package(GMock REQUIRED)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${GMOCK_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR})

enable_testing()

add_subdirectory(test)
