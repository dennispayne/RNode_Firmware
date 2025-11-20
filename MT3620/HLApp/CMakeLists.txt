# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)

project(RNodeHLApp C)

azsphere_configure_tools(TOOLS_REVISION "24.03")
azsphere_configure_api(TARGET_API_SET "16")

add_executable(${PROJECT_NAME}
    main.c
    azure_iot.c
    icm_handler.c
    telemetry_validator.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(${PROJECT_NAME}
    applibs
    pthread
    gcc_s
    c
    azureiot
    iothub_client
    umqtt
    parson
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

azsphere_target_add_image_package(${PROJECT_NAME})
