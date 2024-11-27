cmake_minimum_required(VERSION 3.16)
project(client)


find_package(jsoncpp CONFIG REQUIRED)
include_directories(${JSON_INC_PATH})

set(EXECUTABLE_OUTPUT_PATH ${EXEC_PATH})

add_executable(j_client Client.cpp)
target_link_libraries(j_client jsoncpp_lib)