#pragma once
#include <string>
#include <unordered_map>
#include "lru_cache.h"

class Store {
    std::unordered_map<std::string, std::string> data_;
    LRUCache lru_;
    int max_keys_;

public:
    Store(int max_keys = 0);

    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& out_value);
    int del(const std::string& key);
    int size() const;
    void flush();
};
