# MIT License
#
# Copyright (c) 2021 Alex Vie (silvercircle@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.17)
option(USE_CLANG "build application with clang" ON)
project(fetchweather LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(JSON_BuildTests OFF CACHE INTERNAL "")

# set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(PkgConfig REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)


#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.71.0 COMPONENTS date_time)

add_compile_options(${GLIB2_CFLAGS_OTHER})
link_directories (${GLIB2_LIBRARY_DIRS})
include_directories (${GLIB2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src ${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIR})

add_executable(${PROJECT_NAME} src/pch.h src/loguru/loguru.cpp src/main.cpp
        src/conf.h src/options.h src/options.cpp src/DataHandler_ImplClimaCell.cpp src/DataHandler_ImplClimaCell.h src/utils.cpp src/utils.h src/DataHandler.cpp src/DataHandler.h src/DataHandler_ImplOWM.cpp src/DataHandler_ImplOWM.h)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
target_link_libraries(${PROJECT_NAME} -ldl ${GLIB2_LIBRARIES} ${SQLite3_LIBRARIES} ${CURL_LIBRARIES})

if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(${PROJECT_NAME} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif()


