#include "lru_cache.h"

LRUCache::LRUCache(int max_size) : max_size_(max_size) {}

void LRUCache::touch(const std::string& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        order_.erase(it->second);
    }
    order_.push_front(key);
    map_[key] = order_.begin();
}

void LRUCache::remove(const std::string& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        order_.erase(it->second);
        map_.erase(it);
    }
}

std::string LRUCache::evict() {
    if (order_.empty()) return "";
    std::string key = order_.back();
    map_.erase(key);
    order_.pop_back();
    return key;
}

bool LRUCache::should_evict(int current_size) const {
    return max_size_ > 0 && current_size > max_size_;
}

void LRUCache::clear() {
    order_.clear();
    map_.clear();
}
