cmake_minimum_required(VERSION 3.20)

project(FreeRTOS_MultiTask_Monitor C)


# ---------------------------------------------------------
# FreeRTOS configuration
# ---------------------------------------------------------

add_library(freertos_config INTERFACE)

target_include_directories(
    freertos_config
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# ---------------------------------------------------------
# FreeRTOS configuration options
# ---------------------------------------------------------

set(FREERTOS_PORT MSVC_MINGW CACHE STRING "" FORCE)

set(FREERTOS_HEAP 4 CACHE STRING "" FORCE)


# ---------------------------------------------------------
# Add FreeRTOS Kernel
# ---------------------------------------------------------

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Kernel
)


# ---------------------------------------------------------
# Application executable
# ---------------------------------------------------------

add_executable(
    FreeRTOS_MultiTask_Monitor
    src/main.c
)


# ---------------------------------------------------------
# Include directories
# ---------------------------------------------------------

target_include_directories(
    FreeRTOS_MultiTask_Monitor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# ---------------------------------------------------------
# Link FreeRTOS
# ---------------------------------------------------------

target_link_libraries(
    FreeRTOS_MultiTask_Monitor
    PRIVATE
        freertos_kernel
)