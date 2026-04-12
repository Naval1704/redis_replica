cmake_minimum_required(VERSION 3.10)
project(redis_replica)

add_executable(redis_replica
    src/main.cpp
    src/config.cpp
    src/resp_parser.cpp
    src/server.cpp
    src/store.cpp
    src/lru_cache.cpp
    src/expiry_manager.cpp
    src/command_handler.cpp
    src/pubsub.cpp
    src/aof.cpp
)

target_link_libraries(redis_replica ws2_32)

add_executable(test_client src/test_client.cpp)
target_link_libraries(test_client ws2_32)

add_executable(benchmark_client src/benchmark_client.cpp)
target_link_libraries(benchmark_client ws2_32)
