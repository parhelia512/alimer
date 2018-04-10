#
# Alimer is based on the Turso3D codebase.
# Copyright (c) 2018 Amer Koleci and contributors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Set compiler variable
set ("${CMAKE_CXX_COMPILER_ID}" ON)

# Configure variables
set (ALIMER_URL "https://github.com/amerkoleci/alimer")
set (ALIMER_DESCRIPTION "Alimer is a free lightweight, cross-platform 2D and 3D game engine implemented in C++11 and released under the MIT license. Forked from Turso3D (https://github.com/cadaver/turso3d).")
execute_process (COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/GetAlimerRevision.cmake WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE ALIMER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
string (REGEX MATCH "([^.]+)\\.([^.]+)\\.(.+)" MATCHED "${ALIMER_VERSION}")

# Setup SDK install destinations
if (WIN32)
    set (SCRIPT_EXT .bat)
else ()
    set (SCRIPT_EXT .sh)
endif ()

if (ANDROID)
    # For Android platform, install to a path based on the chosen Android ABI, e.g. libs/armeabi-v7a
    set (LIB_SUFFIX s/${ANDROID_NDK_ABI_NAME})
endif ()

set (DEST_BASE_INCLUDE_DIR include)
set (DEST_INCLUDE_DIR ${DEST_BASE_INCLUDE_DIR}/Alimer)
set (DEST_BIN_DIR bin)
if (WIN32 AND BUILD_SHARED_LIBS)
    # Windows can not find shared libraries in other directories.
    set (DEST_TOOLS_DIR ${DEST_BIN_DIR})
    set (DEST_SAMPLES_DIR ${DEST_BIN_DIR})
else ()
    set (DEST_TOOLS_DIR ${DEST_BIN_DIR}/tools)
    set (DEST_SAMPLES_DIR ${DEST_BIN_DIR}/samples)
endif ()
set (DEST_SHARE_DIR share)
set (DEST_LIBRARY_DIR lib${LIB_SUFFIX})
if (ANDROID)
    set (SHARED_LIB_INSTALL_DIR lib${LIB_SUFFIX})
else ()
    set (SHARED_LIB_INSTALL_DIR bin)
endif ()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_BIN_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${SHARED_LIB_INSTALL_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_LIBRARY_DIR})

macro (add_alimer_executable TARGET)
    file (GLOB SOURCE_FILES *.cpp *.h)
    if (NOT ALIMER_WIN32_CONSOLE)
        set (TARGET_TYPE WIN32)
    endif ()

    if (ANDROID)
        add_library(${TARGET} SHARED ${ARGN})
    else ()
        add_executable (${TARGET} ${TARGET_TYPE} ${ARGN})
    endif ()
    target_link_libraries (${TARGET} Alimer)

endmacro ()