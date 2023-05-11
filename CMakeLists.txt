# Copyright 2014-2023 Jetperch LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 3.1)

project(jsdrv_c_template  # <--- Change to your project name
        VERSION 0.1.0     # <--- Change to your project version
        LANGUAGES C)
SET(VERSION_STRING "${PROJECT_VERSION}")
SET(VERSION_UNDERSCORE "${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")

function (SET_FILENAME _filename)
    get_filename_component(b ${_filename} NAME)
    set_source_files_properties(${_filename} PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${b}\"")
endfunction (SET_FILENAME)


if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall -Wextra -Wpedantic -Werror -fPIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif()
remove_definitions(-D__cplusplus)
if (WIN32 AND CMAKE_COMPILER_IS_GNUCC)
    # Ugh, mingw
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format -std=gnu11")
endif()

add_subdirectory(third-party)
add_subdirectory(src)
