#include "aof.h"
#include "resp_parser.h"
#include <iostream>
#include <sstream>
#include <io.h>
#include <fcntl.h>

AOF::AOF() : enabled_(false) {}

void AOF::init(const std::string& filepath, bool enabled) {
    filepath_ = filepath;
    enabled_ = enabled;
    if (enabled_) {
        file_.open(filepath_, std::ios::app);
        if (!file_.is_open()) {
            std::cout << "Warning: could not open AOF file: " << filepath_ << std::endl;
            enabled_ = false;
        }
    }
}

void AOF::append(const std::vector<std::string>& command) {
    if (!enabled_ || !file_.is_open()) return;

    // write command in RESP format
    RESPParser resp;
    file_ << "*" << command.size() << "\r\n";
    for (const auto& arg : command) {
        file_ << resp.serialize_bulk_string(arg);
    }
    file_.flush();
}

void AOF::replay(std::vector<std::vector<std::string>>& commands) {
    std::ifstream in(filepath_);
    if (!in.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();

    RESPParser parser;
    size_t pos = 0;
    while (pos < content.size()) {
        // find the next complete RESP array
        if (content[pos] != '*') break;

        // find enough \r\n sequences to parse a complete command
        size_t end = pos;
        size_t crln = content.find("\r\n", end);
        if (crln == std::string::npos) break;

        int count = std::stoi(content.substr(end + 1, crln - end - 1));
        end = crln + 2;

        for (int i = 0; i < count; i++) {
            if (end >= content.size() || content[end] != '$') { end = std::string::npos; break; }
            crln = content.find("\r\n", end);
            if (crln == std::string::npos) break;
            int len = std::stoi(content.substr(end + 1, crln - end - 1));
            end = crln + 2 + len + 2;
        }

        if (end == std::string::npos || end > content.size()) break;

        std::string cmd_str = content.substr(pos, end - pos);
        auto parsed = parser.parse(cmd_str);
        if (!parsed.empty()) {
            commands.push_back(parsed);
        }
        pos = end;
    }
}

void AOF::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}

void AOF::close() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}
