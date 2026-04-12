# Redis Replica
A Redis server replica built from scratch in C++ using Winsock on Windows.

## Prerequisites
- CMake 3.10 or higher
- A C++ compiler (MSVC, MinGW, etc.)

## Build
```bash
cd E:/cse/redis_replica
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/Debug/redis_replica.exe
```
