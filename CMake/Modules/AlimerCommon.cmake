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
set (DEST_TOOLS_DIR ${DEST_BIN_DIR})
set (DEST_SAMPLES_DIR ${DEST_BIN_DIR})
set (DEST_SHARE_DIR share)
set (DEST_RESOURCE_DIR ${DEST_BIN_DIR})
set (DEST_BUNDLE_DIR ${DEST_SHARE_DIR}/Applications)
set (DEST_ARCHIVE_DIR lib${LIB_SUFFIX})
set (DEST_THIRDPARTY_HEADERS_DIR ${DEST_INCLUDE_DIR}/ThirdParty)
if (ANDROID)
    set (DEST_LIBRARY_DIR ${DEST_ARCHIVE_DIR})
else ()
    set (DEST_LIBRARY_DIR bin)
endif ()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_BIN_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_LIBRARY_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_ARCHIVE_DIR})


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DALIMER_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DALIMER_DEBUG")

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

# Macro for setting symbolic link on platform that supports it
macro (create_symlink SOURCE DESTINATION)
    # Make absolute paths so they work more reliably on cmake-gui
    if (IS_ABSOLUTE ${SOURCE})
        set (ABS_SOURCE ${SOURCE})
    else ()
        set (ABS_SOURCE ${CMAKE_SOURCE_DIR}/${SOURCE})
    endif ()
    if (IS_ABSOLUTE ${DESTINATION})
        set (ABS_DESTINATION ${DESTINATION})
    else ()
        set (ABS_DESTINATION ${CMAKE_BINARY_DIR}/${DESTINATION})
    endif ()
    if (CMAKE_HOST_WIN32)
        if (IS_DIRECTORY ${ABS_SOURCE})
            set (SLASH_D /D)
        else ()
            unset (SLASH_D)
        endif ()
        set (RESULT_CODE 1)
        if(${CMAKE_SYSTEM_VERSION} GREATER_EQUAL 6.0)
            if (NOT EXISTS ${ABS_DESTINATION})
                # Have to use string-REPLACE as file-TO_NATIVE_PATH does not work as expected with MinGW on "backward slash" host system
                string (REPLACE / \\ BACKWARD_ABS_DESTINATION ${ABS_DESTINATION})
                string (REPLACE / \\ BACKWARD_ABS_SOURCE ${ABS_SOURCE})
                execute_process (COMMAND cmd /C mklink ${SLASH_D} ${BACKWARD_ABS_DESTINATION} ${BACKWARD_ABS_SOURCE} OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE RESULT_CODE)
            endif ()
        endif ()
        if (NOT "${RESULT_CODE}" STREQUAL "0")
            if (SLASH_D)
                set (COMMAND COMMAND ${CMAKE_COMMAND} -E copy_directory ${ABS_SOURCE} ${ABS_DESTINATION})
            else ()
                set (COMMAND COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ABS_SOURCE} ${ABS_DESTINATION})
            endif ()
            # Fallback to copy only one time
            if (NOT EXISTS ${ABS_DESTINATION})
                execute_process (${COMMAND})
            endif ()
        endif ()
    else ()
        execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${ABS_SOURCE} ${ABS_DESTINATION})
    endif ()
endmacro ()

# Groups sources into subfolders.
macro(group_sources)
    file (GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/**)
    foreach (child ${children})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
            string(REPLACE "/" "\\" groupname "${child}")
            file (GLOB files LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*)
            source_group(${groupname} FILES ${files})
        endif ()
    endforeach ()
endmacro()

# Macro for object libraries to "link" to other object libraries. Macro does not actually link them, only pulls in
# interface include directories and defines. Macro operates under assumption that all the object library dependencies
# will eventually end up in one compiled target.
macro (target_link_objects TARGET)
    get_target_property(_MAIN_TARGET_TYPE ${TARGET} TYPE)
    foreach (link_to ${ARGN})
        if (TARGET ${link_to})
            add_dependencies(${TARGET} ${link_to})
            get_target_property(_TARGET_TYPE ${link_to} TYPE)
            if(_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
                get_target_property(LINK_LIBRARIES ${link_to} OBJECT_LINK_LIBRARIES)
                if (LINK_LIBRARIES)
                    target_link_objects(${TARGET} ${LINK_LIBRARIES})
                endif ()

                get_target_property(INCLUDE_DIRECTORIES ${link_to} INTERFACE_INCLUDE_DIRECTORIES)
                if (INCLUDE_DIRECTORIES)
                    get_target_property(TARGET_INCLUDE_DIRECTORIES ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
                    foreach(dir ${INCLUDE_DIRECTORIES})
                        list(FIND TARGET_INCLUDE_DIRECTORIES "${dir}" CONTAINS)
                        if (${CONTAINS} EQUAL -1)
                            target_include_directories(${TARGET} PUBLIC ${INCLUDE_DIRECTORIES})
                        endif ()
                    endforeach()
                endif ()
                get_target_property(COMPILE_DEFINITIONS ${link_to} INTERFACE_COMPILE_DEFINITIONS)
                if (COMPILE_DEFINITIONS)
                    get_target_property(TARGET_INTERFACE_COMPILE_DEFINITIONS ${TARGET} INTERFACE_COMPILE_DEFINITIONS)
                    foreach(def ${COMPILE_DEFINITIONS})
                        list(FIND TARGET_INTERFACE_COMPILE_DEFINITIONS "${def}" CONTAINS)
                        if (${CONTAINS} EQUAL -1)
                            target_compile_definitions(${TARGET} PUBLIC ${COMPILE_DEFINITIONS})
                        endif ()
                    endforeach()
                endif ()
                get_target_property(LINK_LIBRARIES ${TARGET} OBJECT_LINK_LIBRARIES)
                if (NOT LINK_LIBRARIES)
                    unset (LINK_LIBRARIES)
                endif ()
                list (APPEND LINK_LIBRARIES "${link_to}")
                list (REMOVE_DUPLICATES LINK_LIBRARIES)
                set_target_properties(${TARGET} PROPERTIES OBJECT_LINK_LIBRARIES "${LINK_LIBRARIES}")
            elseif (NOT _MAIN_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
                target_link_libraries(${TARGET} ${link_to})
            endif ()
        else ()
            if ("${_MAIN_TARGET_TYPE}" STREQUAL "OBJECT_LIBRARY")
                get_target_property(LINK_LIBRARIES ${TARGET} OBJECT_LINK_LIBRARIES)
                if (NOT LINK_LIBRARIES)
                    unset (LINK_LIBRARIES)
                endif ()
                foreach (link ${ARGN})
                    list (APPEND LINK_LIBRARIES ${link})
                endforeach()
                list(REMOVE_DUPLICATES LINK_LIBRARIES)
                set_target_properties(${TARGET} PROPERTIES OBJECT_LINK_LIBRARIES "${LINK_LIBRARIES}")
            else ()
                target_link_libraries(${TARGET} ${link_to})
            endif ()
        endif ()
    endforeach()
endmacro()