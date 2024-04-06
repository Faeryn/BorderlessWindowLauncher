cmake_minimum_required(VERSION 3.21)
project(NWNLauncher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(OUTPUT_DIR_APP "${CMAKE_BINARY_DIR}/../bin/app")

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "${OUTPUT_DIR_APP}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${OUTPUT_DIR_APP}/nwnlauncher.exe")