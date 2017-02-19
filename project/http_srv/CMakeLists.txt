cmake_minimum_required(VERSION 2.8)

project(http_serv)

set(CMAKE_CXX_COMPILER "g++")

set(CFLAGS_WARN "-Wall -Wextra -Werror")
set(CFLAGS_REL  "-O2")
set(CFLAGS_DBG  "-O0 -ggdb3")

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++14 -DBOOST_LOG_DYN_LINK ${CFLAGS_WARN}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}    ${CFLAGS_DBG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  ${CFLAGS_REL}")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, using Release as default")
    set(CMAKE_BUILD_TYPE "Release")
endif()

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} pthread boost_system boost_log boost_thread boost_regex)
