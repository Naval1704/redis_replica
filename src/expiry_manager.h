#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <chrono>

class ExpiryManager {
    struct ExpiryEntry {
        std::string key;
        std::chrono::steady_clock::time_point expires_at;

        bool operator>(const ExpiryEntry& other) const {
            return expires_at > other.expires_at;
        }
    };

    std::priority_queue<ExpiryEntry, std::vector<ExpiryEntry>, std::greater<ExpiryEntry>> heap_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map_;

public:
    void set_expiry(const std::string& key, int seconds);
    bool is_expired(const std::string& key) const;
    int ttl(const std::string& key) const;  // returns -1 if no expiry, -2 if not found
    bool has_expiry(const std::string& key) const;
    void remove(const std::string& key);
    void cleanup(std::vector<std::string>& expired_keys);
    void clear();
};
