#pragma once // without it if two files include this header, it will cause redefinition error
#include <unordered_map>
#include <string>

class Config {
    std::unordered_map<std::string, std::string> config_map;
    public:
    bool load_config(const std::string& filePath);
    std::string get_config_value(const std::string& key);
};