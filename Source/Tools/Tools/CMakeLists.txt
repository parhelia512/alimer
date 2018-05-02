#
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

file (GLOB_RECURSE HEADER_FILES *.h *.hpp)
file (GLOB_RECURSE SOURCE_FILES *.cpp)

# Group source code in VS solution
group_sources()

add_library (AlimerAssets ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (AlimerAssets Alimer)

target_include_directories(AlimerAssets
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE
		$<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>
)

install (FILES ${HEADER_FILES} DESTINATION ${DEST_INCLUDE_DIR}/${path})
install(TARGETS AlimerAssets
	EXPORT AlimerAssets
	LIBRARY DESTINATION ${DEST_LIBRARY_DIR}
	RUNTIME DESTINATION ${DEST_BIN_DIR}
	ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR}
)

set_property(TARGET AlimerAssets PROPERTY FOLDER "Tools")