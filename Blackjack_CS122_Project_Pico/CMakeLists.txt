# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(CS122_Project C CXX ASM)

pico_sdk_init()

add_executable(CS122_Project 
CS122_Project.cpp 
http_server.cpp  
lvgl_BlackjackApp.cpp 
cs122_app.cpp 
spi_display.cpp
Card.cpp
card_back.c
card_font.c
card_font_small.c
chips.c
)

pico_set_program_name(CS122_Project "CS122_Project")
pico_set_program_version(CS122_Project "0.1")

pico_enable_stdio_uart(CS122_Project 0)
pico_enable_stdio_usb(CS122_Project 1)

add_subdirectory(lvgl)

target_include_directories(CS122_Project PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

add_library(dhcpserver
    dhcpserver/dhcpserver.c
)

target_include_directories(dhcpserver PUBLIC
    dhcpserver
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(dhcpserver
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)

target_include_directories(CS122_Project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    dhcpserver
)

target_link_libraries(CS122_Project
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    dhcpserver
    hardware_spi
    lvgl
    pico_rand 
)

pico_add_extra_outputs(CS122_Project)