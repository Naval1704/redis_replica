#include "expiry_manager.h"
using namespace std::chrono;

void ExpiryManager::set_expiry(const std::string& key, int seconds) {
    auto expires_at = steady_clock::now() + std::chrono::seconds(seconds);
    expiry_map_[key] = expires_at;
    heap_.push({key, expires_at});
}

bool ExpiryManager::is_expired(const std::string& key) const {
    auto it = expiry_map_.find(key);
    if (it == expiry_map_.end()) return false;
    return steady_clock::now() >= it->second;
}

int ExpiryManager::ttl(const std::string& key) const {
    auto it = expiry_map_.find(key);
    if (it == expiry_map_.end()) return -1;
    auto remaining = duration_cast<std::chrono::seconds>(it->second - steady_clock::now()).count();
    if (remaining <= 0) return -2;
    return (int)remaining;
}

bool ExpiryManager::has_expiry(const std::string& key) const {
    return expiry_map_.find(key) != expiry_map_.end();
}

void ExpiryManager::remove(const std::string& key) {
    expiry_map_.erase(key);
}

void ExpiryManager::cleanup(std::vector<std::string>& expired_keys) {
    auto now = steady_clock::now();
    while (!heap_.empty()) {
        const auto& top = heap_.top();
        auto it = expiry_map_.find(top.key);
        // lazy invalidation: skip entries that were removed or updated
        if (it == expiry_map_.end() || it->second != top.expires_at) {
            heap_.pop();
            continue;
        }
        if (top.expires_at > now) break;
        expired_keys.push_back(top.key);
        expiry_map_.erase(it);
        heap_.pop();
    }
}

void ExpiryManager::clear() {
    expiry_map_.clear();
    while (!heap_.empty()) heap_.pop();
}
