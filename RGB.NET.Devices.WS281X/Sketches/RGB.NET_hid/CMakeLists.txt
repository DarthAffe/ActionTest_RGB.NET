cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(RGB.NET)

pico_sdk_init()

add_executable(RGB.NET)

pico_generate_pio_header(RGB.NET ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_include_directories(RGB.NET PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_sources(RGB.NET PRIVATE RGB.NET.c usb_descriptors.c)

target_link_libraries(RGB.NET PRIVATE pico_stdlib hardware_pio pico_multicore tinyusb_device tinyusb_board)

pico_add_extra_outputs(RGB.NET)
