# Redis Replica

A Redis server replica built from scratch in C++ using Winsock on Windows. Implements the RESP2 protocol with a non-blocking select() event loop, LRU eviction, TTL expiration, pub/sub, and AOF persistence.

## Supported Commands

| Command | Syntax | Description |
|---------|--------|-------------|
| PING | `PING` | Returns PONG |
| SET | `SET key value [EX seconds]` | Set a key-value pair, optionally with expiry |
| GET | `GET key` | Get value by key |
| DEL | `DEL key [key ...]` | Delete one or more keys |
| EXPIRE | `EXPIRE key seconds` | Set TTL on a key |
| TTL | `TTL key` | Get remaining TTL (-1 = no expiry, -2 = not found) |
| DBSIZE | `DBSIZE` | Return number of keys |
| FLUSHDB | `FLUSHDB` | Delete all keys |
| SUBSCRIBE | `SUBSCRIBE channel [channel ...]` | Subscribe to channels |
| UNSUBSCRIBE | `UNSUBSCRIBE [channel ...]` | Unsubscribe from channels |
| PUBLISH | `PUBLISH channel message` | Publish a message to a channel |

## Prerequisites

- CMake 3.10 or higher
- A C++ compiler (MSVC, MinGW, etc.)
- Windows (uses Winsock2)

## Build

```bash
cd project_path
cmake -S . -B build
cmake --build build
```

## Run

Start the server:
```bash
./build/Debug/redis_replica.exe
```

The server reads `redis.conf` for configuration (port, maxmemory, AOF settings).

## Testing

### Run the test client

With the server running in one terminal, open a second terminal and run:
```bash
./build/Debug/test_client.exe
```

This runs automated tests for all supported commands and prints PASS/FAIL for each.

### Manual testing with PowerShell

With the server running, open PowerShell and connect:
```powershell
$c = New-Object System.Net.Sockets.TcpClient("127.0.0.1", 6379)
$s = $c.GetStream()

# Send a PING command
$data = [System.Text.Encoding]::ASCII.GetBytes("*1`r`n`$4`r`nPING`r`n")
$s.Write($data, 0, $data.Length)

# Read response
$buf = New-Object byte[] 1024
$n = $s.Read($buf, 0, $buf.Length)
[System.Text.Encoding]::ASCII.GetString($buf, 0, $n)
# Expected: +PONG

$c.Close()
```

### Testing with redis-cli

If you have `redis-cli` installed, you can connect directly:
```bash
redis-cli -p 6379
> PING
PONG
> SET name Gaurav
OK
> GET name
"Gaurav"
```

## Configuration

Edit `redis.conf` to change settings:

```
bind 127.0.0.1
port 6379
maxmemory 256
appendonly yes
appendfilename appendonly.aof
```

- `port` - server port (default 6379)
- `maxmemory` - max number of keys before LRU eviction kicks in
- `appendonly` - enable/disable AOF persistence (yes/no)
- `appendfilename` - AOF log file path

## Architecture

```
src/
  main.cpp              - Entry point, Winsock init, config loading
  config.h/cpp          - Config file parser
  resp_parser.h/cpp     - RESP2 protocol parser and serializer
  server.h/cpp          - TCP server with non-blocking select() event loop
  store.h/cpp           - Key-value hash map
  lru_cache.h/cpp       - LRU eviction (doubly linked list + hash map)
  expiry_manager.h/cpp  - TTL management with min-heap
  command_handler.h/cpp - Command dispatch and execution
  pubsub.h/cpp          - Publish/Subscribe channels
  aof.h/cpp             - Append-only file persistence and replay
```
