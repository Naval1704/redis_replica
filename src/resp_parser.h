#pragma once 
#include <string> 
#include <vector> 

class RESPParser {
    public:
    std::string serialize_simple_string(const std::string& str);
    std::string serialize_error(const std::string& str);
    std::string serialize_integer(int num);
    std::string serialize_bulk_string(const std::string& str);
    std::string serialize_null();
    std::vector<std::string> parse(std::string command);
};