cmake_minimum_required(VERSION 3.13)

option(NATIVE_BUILD "Build a native (non-Pico) version of the harness for local testing" OFF)

set(EMBEDDED_TASK_SOURCES
    src/main.c
    src/logic.c
    src/mock_hw.c
    test/fault_scenarios.c
)

set(EMBEDDED_TASK_INCLUDE_DIRS
    include
    test
)

if (NATIVE_BUILD)
    project(embedded_task C)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    add_executable(embedded_task ${EMBEDDED_TASK_SOURCES})
    target_include_directories(embedded_task PRIVATE ${EMBEDDED_TASK_INCLUDE_DIRS})
else ()
    include(pico_sdk_import.cmake)

    project(embedded_task C CXX ASM)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 17)

    pico_sdk_init()

    add_executable(embedded_task ${EMBEDDED_TASK_SOURCES})
    target_include_directories(embedded_task PRIVATE ${EMBEDDED_TASK_INCLUDE_DIRS})
    target_compile_definitions(embedded_task PRIVATE PICO_BUILD)
    target_link_libraries(embedded_task PRIVATE pico_stdlib)

    pico_enable_stdio_usb(embedded_task 1)
    pico_enable_stdio_uart(embedded_task 0)

    pico_add_extra_outputs(embedded_task)
endif ()
