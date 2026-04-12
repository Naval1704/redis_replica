#pragma once
#include <string>
#include <vector>
#include <fstream>

class AOF {
    std::string filepath_;
    std::ofstream file_;
    bool enabled_;

public:
    AOF();
    void init(const std::string& filepath, bool enabled);
    void append(const std::vector<std::string>& command);
    void replay(std::vector<std::vector<std::string>>& commands);
    void flush();
    void close();
};
