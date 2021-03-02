cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(RGB.NET)

pico_sdk_init()

add_executable(RGB.NET)

pico_generate_pio_header(RGB.NET ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_sources(RGB.NET PRIVATE RGB.NET.c)

target_link_libraries(RGB.NET PRIVATE pico_stdlib hardware_pio pico_multicore)

pico_enable_stdio_usb(RGB.NET 1)
pico_enable_stdio_uart(RGB.NET 0)

pico_add_extra_outputs(RGB.NET)