# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)

project(RNodeRTApp C)

azsphere_configure_tools(TOOLS_REVISION "24.03")
azsphere_configure_api(TARGET_API_SET "16")

add_executable(${PROJECT_NAME} main.c rnode_core.c icm_handler.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common)

target_link_libraries(${PROJECT_NAME} applibs gcc_s c)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

azsphere_target_add_image_package(${PROJECT_NAME})
