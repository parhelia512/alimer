set (TARGET_NAME spirv_cross)

set(SPIRV_CROSS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
)

if (ALIMER_D3D11 OR ALIMER_D3D12)
    list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.hpp
    )
endif()

if (ALIMER_VULKAN OR ALIMER_OPENGL)
    list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp
    )
endif()

add_library(${TARGET_NAME} STATIC ${SPIRV_CROSS_SOURCES})
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "ThirdParty")