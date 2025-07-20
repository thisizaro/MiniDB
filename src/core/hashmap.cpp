/**
 * @file hashmap.cpp
 * @brief HashMap implementation
 */

#include "minidb/core/hashmap.h"
#include <algorithm>

namespace minidb {
namespace core {

    // Explicit template instantiations
    template class HashMap<std::string, std::string>;
    template class HashMap<std::string, int>;
    template class HashMap<int, std::string>;
    template class HashMap<int, int>;
    
    // HashMap Iterator implementation
    template<typename K, typename V>
    HashMap<K, V>::Iterator::Iterator(
        typename std::vector<Bucket>::iterator bucket_it,
        typename std::vector<Bucket>::iterator bucket_end,
        typename Bucket::iterator entry_it)
        : bucket_it_(bucket_it), bucket_end_(bucket_end), entry_it_(entry_it) {
        advance_to_next_valid();
    }
    
    template<typename K, typename V>
    void HashMap<K, V>::Iterator::advance_to_next_valid() {
        while (bucket_it_ != bucket_end_) {
            if (bucket_it_->empty()) {
                ++bucket_it_;
                if (bucket_it_ != bucket_end_) {
                    entry_it_ = bucket_it_->begin();
                }
            } else if (entry_it_ == bucket_it_->end()) {
                ++bucket_it_;
                if (bucket_it_ != bucket_end_) {
                    entry_it_ = bucket_it_->begin();
                }
            } else {
                break;  // Found valid entry
            }
        }
    }
    
    template<typename K, typename V>
    auto HashMap<K, V>::Iterator::operator++() -> Iterator& {
        if (bucket_it_ != bucket_end_ && entry_it_ != bucket_it_->end()) {
            ++entry_it_;
            advance_to_next_valid();
        }
        return *this;
    }
    
    template<typename K, typename V>
    auto HashMap<K, V>::Iterator::operator++(int) -> Iterator {
        Iterator temp = *this;
        ++(*this);
        return temp;
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::Iterator::operator==(const Iterator& other) const {
        return bucket_it_ == other.bucket_it_ && 
               (bucket_it_ == bucket_end_ || entry_it_ == other.entry_it_);
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::Iterator::operator!=(const Iterator& other) const {
        return !(*this == other);
    }
    
    // HashMap main implementation
    template<typename K, typename V>
    bool HashMap<K, V>::insert(const K& key, const V& value) {
        // Check if key already exists
        if (find(key) != nullptr) {
            return false;  // Key already exists
        }
        
        // Check if rehash is needed
        if (needs_rehash()) {
            rehash(bucket_count_ * 2);
        }
        
        size_t bucket_index = hash(key);
        buckets_[bucket_index].emplace_back(key, value);
        size_++;
        
        return true;
    }
    
    template<typename K, typename V>
    V* HashMap<K, V>::find(const K& key) {
        size_t bucket_index = hash(key);
        Bucket& bucket = buckets_[bucket_index];
        
        for (auto& entry : bucket) {
            if (equal_func_(entry.key, key)) {
                return &entry.value;
            }
        }
        
        return nullptr;
    }
    
    template<typename K, typename V>
    const V* HashMap<K, V>::find(const K& key) const {
        size_t bucket_index = hash(key);
        const Bucket& bucket = buckets_[bucket_index];
        
        for (const auto& entry : bucket) {
            if (equal_func_(entry.key, key)) {
                return &entry.value;
            }
        }
        
        return nullptr;
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::remove(const K& key) {
        size_t bucket_index = hash(key);
        Bucket& bucket = buckets_[bucket_index];
        
        auto it = std::find_if(bucket.begin(), bucket.end(),
            [&](const Entry& entry) {
                return equal_func_(entry.key, key);
            });
        
        if (it != bucket.end()) {
            bucket.erase(it);
            size_--;
            return true;
        }
        
        return false;
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::update(const K& key, const V& value) {
        V* existing = find(key);
        if (existing != nullptr) {
            *existing = value;
            return true;
        }
        return false;
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::upsert(const K& key, const V& value) {
        V* existing = find(key);
        if (existing != nullptr) {
            *existing = value;
            return false;  // Updated existing
        } else {
            insert(key, value);
            return true;  // Inserted new
        }
    }
    
    template<typename K, typename V>
    bool HashMap<K, V>::contains(const K& key) const {
        return find(key) != nullptr;
    }
    
    template<typename K, typename V>
    void HashMap<K, V>::clear() {
        for (auto& bucket : buckets_) {
            bucket.clear();
        }
        size_ = 0;
    }
    
    template<typename K, typename V>
    auto HashMap<K, V>::begin() -> Iterator {
        auto bucket_it = buckets_.begin();
        auto bucket_end = buckets_.end();
        
        if (bucket_it != bucket_end) {
            return Iterator(bucket_it, bucket_end, bucket_it->begin());
        }
        
        return Iterator(bucket_end, bucket_end, Bucket::iterator{});
    }
    
    template<typename K, typename V>
    auto HashMap<K, V>::end() -> Iterator {
        return Iterator(buckets_.end(), buckets_.end(), Bucket::iterator{});
    }
    
    template<typename K, typename V>
    V& HashMap<K, V>::operator[](const K& key) {
        V* existing = find(key);
        if (existing != nullptr) {
            return *existing;
        }
        
        // Insert default value and return reference
        insert(key, V{});
        return *find(key);
    }
    
    template<typename K, typename V>
    void HashMap<K, V>::rehash(size_t new_bucket_count) {
        if (new_bucket_count <= bucket_count_) {
            return;  // No need to shrink
        }
        
        // Save old buckets
        std::vector<Bucket> old_buckets = std::move(buckets_);
        
        // Reinitialize with new size
        bucket_count_ = new_bucket_count;
        buckets_.clear();
        buckets_.resize(bucket_count_);
        size_ = 0;
        
        // Reinsert all entries
        for (const auto& bucket : old_buckets) {
            for (const auto& entry : bucket) {
                insert(entry.key, entry.value);
            }
        }
    }
    
    template<typename K, typename V>
    void HashMap<K, V>::print_stats() const {
        std::cout << "HashMap Statistics:" << std::endl;
        std::cout << "  Size: " << size_ << std::endl;
        std::cout << "  Bucket Count: " << bucket_count_ << std::endl;
        std::cout << "  Load Factor: " << load_factor() << std::endl;
        
        size_t empty_buckets = 0;
        size_t max_bucket_size = 0;
        
        for (const auto& bucket : buckets_) {
            if (bucket.empty()) {
                empty_buckets++;
            }
            max_bucket_size = std::max(max_bucket_size, bucket.size());
        }
        
        std::cout << "  Empty Buckets: " << empty_buckets << std::endl;
        std::cout << "  Max Bucket Size: " << max_bucket_size << std::endl;
    }

} // namespace core
} // namespace minidb
