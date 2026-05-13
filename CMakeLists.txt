cmake_minimum_required(VERSION 3.20.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include(ApplySourceList)

project(
    asb-box
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SourcesC "")
add_subdirectory(src)

target_sources(app PRIVATE ${SourcesC})
target_include_directories(app PRIVATE src)