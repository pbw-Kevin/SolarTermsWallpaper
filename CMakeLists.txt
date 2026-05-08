cmake_minimum_required(VERSION 3.29.1)

project(SolarTermsWallpaper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(/source-charset:utf-8)
add_compile_options(/execution-charset:gb2312)
add_compile_options(/MT)
aux_source_directory(. CPP_LIST)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
add_executable(SolarTermsWallpaper main.cpp favicon.rc)