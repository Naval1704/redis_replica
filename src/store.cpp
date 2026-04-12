#include "store.h"

Store::Store(int max_keys) : max_keys_(max_keys), lru_(max_keys) {}

void Store::set(const std::string& key, const std::string& value) {
    data_[key] = value;
    lru_.touch(key);

    while (lru_.should_evict((int)data_.size())) {
        std::string evicted = lru_.evict();
        if (!evicted.empty()) {
            data_.erase(evicted);
        }
    }
}

bool Store::get(const std::string& key, std::string& out_value) {
    auto it = data_.find(key);
    if (it == data_.end()) return false;
    out_value = it->second;
    lru_.touch(key);
    return true;
}

int Store::del(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) return 0;
    data_.erase(it);
    lru_.remove(key);
    return 1;
}

int Store::size() const {
    return (int)data_.size();
}

void Store::flush() {
    data_.clear();
    lru_.clear();
}
