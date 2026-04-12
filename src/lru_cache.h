#pragma once
#include <string>
#include <list>
#include <unordered_map>

class LRUCache {
    int max_size_;
    std::list<std::string> order_;  // front = most recent, back = least recent
    std::unordered_map<std::string, std::list<std::string>::iterator> map_;

public:
    LRUCache(int max_size = 0);

    void touch(const std::string& key);
    void remove(const std::string& key);
    std::string evict();  // removes and returns the least recently used key
    bool should_evict(int current_size) const;
    void clear();
};
