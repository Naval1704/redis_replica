#define FD_SETSIZE 1024
#include <WinSock2.h>
#include <iostream>
#include <string>
using namespace std;

SOCKET sock;

string send_command(const string& cmd) {
    send(sock, cmd.c_str(), (int)cmd.size(), 0);
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return "(no response)";
    buffer[bytes] = '\0';
    return string(buffer, bytes);
}

// helper to build RESP array from args
string make_resp(initializer_list<string> args) {
    string result = "*" + to_string(args.size()) + "\r\n";
    for (const auto& arg : args) {
        result += "$" + to_string(arg.size()) + "\r\n" + arg + "\r\n";
    }
    return result;
}

void test(const string& name, const string& command, const string& expected) {
    string response = send_command(command);
    bool pass = response == expected;
    cout << (pass ? "[PASS] " : "[FAIL] ") << name;
    if (!pass) {
        cout << " | expected: ";
        for (char c : expected) {
            if (c == '\r') cout << "\\r";
            else if (c == '\n') cout << "\\n";
            else cout << c;
        }
        cout << " | got: ";
        for (char c : response) {
            if (c == '\r') cout << "\\r";
            else if (c == '\n') cout << "\\n";
            else cout << c;
        }
    }
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
    cout << "Connected to server. Running tests...\n" << endl;

    // PING
    test("PING", make_resp({"PING"}), "+PONG\r\n");

    // SET and GET
    test("SET name", make_resp({"SET", "name", "Gaurav"}), "+OK\r\n");
    test("GET name", make_resp({"GET", "name"}), "$6\r\nGaurav\r\n");

    // GET missing key
    test("GET missing", make_resp({"GET", "nokey"}), "$-1\r\n");

    // SET with overwrite
    test("SET name overwrite", make_resp({"SET", "name", "Naval"}), "+OK\r\n");
    test("GET name after overwrite", make_resp({"GET", "name"}), "$5\r\nNaval\r\n");

    // DEL
    test("DEL name", make_resp({"DEL", "name"}), ":1\r\n");
    test("GET after DEL", make_resp({"GET", "name"}), "$-1\r\n");
    test("DEL missing", make_resp({"DEL", "nokey"}), ":0\r\n");

    // Multiple SET for DBSIZE
    test("SET a", make_resp({"SET", "a", "1"}), "+OK\r\n");
    test("SET b", make_resp({"SET", "b", "2"}), "+OK\r\n");
    test("SET c", make_resp({"SET", "c", "3"}), "+OK\r\n");
    test("DBSIZE", make_resp({"DBSIZE"}), ":3\r\n");

    // EXPIRE and TTL
    test("SET temp", make_resp({"SET", "temp", "value"}), "+OK\r\n");
    test("EXPIRE temp 10", make_resp({"EXPIRE", "temp", "10"}), ":1\r\n");
    test("TTL temp", make_resp({"TTL", "temp"}), ":9\r\n");  // might be 10 or 9
    test("TTL missing", make_resp({"TTL", "nokey"}), ":-2\r\n");
    test("TTL no expiry", make_resp({"TTL", "a"}), ":-1\r\n");

    // SET with EX
    test("SET ex", make_resp({"SET", "quick", "val", "EX", "5"}), "+OK\r\n");
    test("TTL ex", make_resp({"TTL", "quick"}), ":4\r\n");  // might be 5 or 4

    // FLUSHDB
    test("FLUSHDB", make_resp({"FLUSHDB"}), "+OK\r\n");
    test("DBSIZE after flush", make_resp({"DBSIZE"}), ":0\r\n");

    // Unknown command
    test("Unknown cmd", make_resp({"FOOBAR"}), "-ERR unknown command 'FOOBAR'\r\n");

    // PUBLISH (no subscribers)
    test("PUBLISH no subs", make_resp({"PUBLISH", "ch1", "hello"}), ":0\r\n");

    cout << "\nDone!" << endl;

    closesocket(sock);
    WSACleanup();
    return 0;
}
