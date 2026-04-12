cmake_minimum_required(VERSION 3.10)
project(redis_replica)
add_executable(redis_replica src/main.cpp src/config.cpp src/resp_parser.cpp) # will take main.cpp as source file and complie it

target_link_libraries(redis_replica ws2_32)