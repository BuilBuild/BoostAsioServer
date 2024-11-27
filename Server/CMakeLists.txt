cmake_minimum_required(VERSION 3.16)
project(Server)


aux_source_directory(./ SRC)
set(EXECUTABLE_OUTPUT_PATH ${EXEC_PATH})

find_package(jsoncpp CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 11)
include_directories(${JSON_INC_PATH})

