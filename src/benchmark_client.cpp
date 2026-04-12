#define FD_SETSIZE 1024
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;
using namespace chrono;

SOCKET sock;

string make_resp(initializer_list<string> args) {
    string result = "*" + to_string(args.size()) + "\r\n";
    for (const auto& arg : args) {
        result += "$" + to_string(arg.size()) + "\r\n" + arg + "\r\n";
    }
    return result;
}

string send_command(const string& cmd) {
    send(sock, cmd.c_str(), (int)cmd.size(), 0);
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return "";
    return string(buffer, bytes);
}

struct BenchResult {
    string name;
    int ops;
    double total_ms;
    double ops_per_sec;
    double avg_latency_us;
    double min_latency_us;
    double max_latency_us;
    double p50_us;
    double p99_us;
};

BenchResult run_benchmark(const string& name, int count, vector<string>& commands) {
    vector<double> latencies;
    latencies.reserve(count);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < count; i++) {
        auto t1 = high_resolution_clock::now();
        send_command(commands[i % commands.size()]);
        auto t2 = high_resolution_clock::now();
        latencies.push_back((double)duration_cast<microseconds>(t2 - t1).count());
    }

    auto end = high_resolution_clock::now();
    double total_ms = duration_cast<microseconds>(end - start).count() / 1000.0;

    sort(latencies.begin(), latencies.end());

    double sum = 0;
    for (double l : latencies) sum += l;

    BenchResult r;
    r.name = name;
    r.ops = count;
    r.total_ms = total_ms;
    r.ops_per_sec = count / (total_ms / 1000.0);
    r.avg_latency_us = sum / count;
    r.min_latency_us = latencies.front();
    r.max_latency_us = latencies.back();
    r.p50_us = latencies[count / 2];
    r.p99_us = latencies[(int)(count * 0.99)];
    return r;
}

void print_result(const BenchResult& r) {
    cout << "=== " << r.name << " ===" << endl;
    cout << "  Operations:    " << r.ops << endl;
    cout << "  Total time:    " << r.total_ms << " ms" << endl;
    cout << "  Throughput:    " << (int)r.ops_per_sec << " ops/sec" << endl;
    cout << "  Avg latency:   " << r.avg_latency_us << " us" << endl;
    cout << "  Min latency:   " << r.min_latency_us << " us" << endl;
    cout << "  Max latency:   " << r.max_latency_us << " us" << endl;
    cout << "  P50 latency:   " << r.p50_us << " us" << endl;
    cout << "  P99 latency:   " << r.p99_us << " us" << endl;
    cout << endl;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(6379);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Could not connect to server. Is it running?" << endl;
        return 1;
    }

    int N = 10000;
    cout << "Running benchmarks with " << N << " operations each...\n" << endl;

    // flush before starting
    send_command(make_resp({"FLUSHDB"}));

    // PING benchmark
    {
        vector<string> cmds = { make_resp({"PING"}) };
        auto r = run_benchmark("PING", N, cmds);
        print_result(r);
    }

    // SET benchmark
    {
        vector<string> cmds;
        for (int i = 0; i < N; i++) {
            cmds.push_back(make_resp({"SET", "key:" + to_string(i), "value:" + to_string(i)}));
        }
        auto r = run_benchmark("SET", N, cmds);
        print_result(r);
    }

    // GET benchmark (keys already set above)
    {
        vector<string> cmds;
        for (int i = 0; i < N; i++) {
            cmds.push_back(make_resp({"GET", "key:" + to_string(i)}));
        }
        auto r = run_benchmark("GET", N, cmds);
        print_result(r);
    }

    // GET missing keys
    {
        vector<string> cmds;
        for (int i = 0; i < N; i++) {
            cmds.push_back(make_resp({"GET", "missing:" + to_string(i)}));
        }
        auto r = run_benchmark("GET (miss)", N, cmds);
        print_result(r);
    }

    // SET with EX
    {
        vector<string> cmds;
        for (int i = 0; i < N; i++) {
            cmds.push_back(make_resp({"SET", "expkey:" + to_string(i), "val", "EX", "60"}));
        }
        auto r = run_benchmark("SET with EX", N, cmds);
        print_result(r);
    }

    // DEL benchmark
    {
        vector<string> cmds;
        for (int i = 0; i < N; i++) {
            cmds.push_back(make_resp({"DEL", "key:" + to_string(i)}));
        }
        auto r = run_benchmark("DEL", N, cmds);
        print_result(r);
    }

    // DBSIZE benchmark
    {
        vector<string> cmds = { make_resp({"DBSIZE"}) };
        auto r = run_benchmark("DBSIZE", N, cmds);
        print_result(r);
    }

    // Summary
    cout << "Benchmark complete. Server had maxmemory set to 256 keys." << endl;

    send_command(make_resp({"FLUSHDB"}));
    closesocket(sock);
    WSACleanup();
    return 0;
}
