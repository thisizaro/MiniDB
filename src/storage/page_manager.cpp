/**
 * @file page_manager.cpp
 * @brief Page Manager implementation
 */

#include "minidb/storage/page_manager.h"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace minidb {
namespace storage {

    // Page implementation
    Page::Page(PageId id, PageSize size) 
        : id_(id), size_(size), dirty_(false), in_use_(false), ref_count_(0) {
        data_.resize(size_, 0);
    }
    
    void Page::clear() {
        std::fill(data_.begin(), data_.end(), 0);
        dirty_ = false;
    }
    
    bool Page::write(size_t offset, const void* data, size_t length) {
        if (offset + length > size_) {
            return false;  // Write would exceed page boundary
        }
        
        std::memcpy(data_.data() + offset, data, length);
        dirty_ = true;
        return true;
    }
    
    bool Page::read(size_t offset, void* data, size_t length) const {
        if (offset + length > size_) {
            return false;  // Read would exceed page boundary
        }
        
        std::memcpy(data, data_.data() + offset, length);
        return true;
    }
    
    // LRU Policy implementation
    PageId LRUPolicy::select_victim(const std::vector<PageId>& pages) {
        if (pages.empty()) {
            return INVALID_PAGE_ID;
        }
        
        // Find the least recently used page
        PageId lru_page = INVALID_PAGE_ID;
        auto lru_position = access_order_.end();
        
        for (PageId page_id : pages) {
            auto it = std::find(access_order_.begin(), access_order_.end(), page_id);
            if (it != access_order_.end()) {
                if (lru_position == access_order_.end() || it < lru_position) {
                    lru_position = it;
                    lru_page = page_id;
                }
            }
        }
        
        return lru_page != INVALID_PAGE_ID ? lru_page : pages[0];
    }
    
    void LRUPolicy::page_accessed(PageId page_id) {
        // Remove if already in list
        auto it = std::find(access_order_.begin(), access_order_.end(), page_id);
        if (it != access_order_.end()) {
            access_order_.erase(it);
        }
        
        // Add to end (most recently used)
        access_order_.push_back(page_id);
    }
    
    void LRUPolicy::page_added(PageId page_id) {
        page_accessed(page_id);  // Same as access for LRU
    }
    
    void LRUPolicy::page_removed(PageId page_id) {
        auto it = std::find(access_order_.begin(), access_order_.end(), page_id);
        if (it != access_order_.end()) {
            access_order_.erase(it);
        }
    }
    
    // Page Manager implementation
    PageManager::PageManager(PageSize page_size, size_t max_pages)
        : replacement_policy_(std::make_unique<LRUPolicy>()),
          page_size_(page_size), max_pages_(max_pages), 
          current_pages_(0), next_page_id_(1) {
    }
    
    PageManager::~PageManager() {
        flush_all();
    }
    
    PageId PageManager::allocate_page() {
        // Check if we need to evict pages
        if (current_pages_ >= max_pages_) {
            if (!evict_pages(1)) {
                return INVALID_PAGE_ID;  // Could not make room
            }
        }
        
        PageId new_page_id = next_page_id_++;
        auto new_page = std::make_unique<Page>(new_page_id, page_size_);
        new_page->set_in_use(true);
        
        pages_[new_page_id] = std::move(new_page);
        current_pages_++;
        
        replacement_policy_->page_added(new_page_id);
        
        return new_page_id;
    }
    
    bool PageManager::deallocate_page(PageId page_id) {
        auto it = pages_.find(page_id);
        if (it == pages_.end()) {
            return false;  // Page not found
        }
        
        // Don't deallocate if page is pinned (has references)
        if (it->second->get_ref_count() > 0) {
            return false;
        }
        
        replacement_policy_->page_removed(page_id);
        pages_.erase(it);
        current_pages_--;
        
        return true;
    }
    
    Page* PageManager::get_page(PageId page_id) {
        auto it = pages_.find(page_id);
        if (it == pages_.end()) {
            return nullptr;
        }
        
        replacement_policy_->page_accessed(page_id);
        return it->second.get();
    }
    
    bool PageManager::pin_page(PageId page_id) {
        Page* page = get_page(page_id);
        if (page == nullptr) {
            return false;
        }
        
        page->add_ref();
        return true;
    }
    
    bool PageManager::unpin_page(PageId page_id) {
        Page* page = get_page(page_id);
        if (page == nullptr) {
            return false;
        }
        
        page->release_ref();
        return true;
    }
    
    bool PageManager::flush_all() {
        bool success = true;
        
        for (const auto& [page_id, page] : pages_) {
            if (page->is_dirty()) {
                // In a real implementation, this would write to disk
                page->mark_clean();
            }
        }
        
        return success;
    }
    
    bool PageManager::flush_page(PageId page_id) {
        Page* page = get_page(page_id);
        if (page == nullptr) {
            return false;
        }
        
        if (page->is_dirty()) {
            // In a real implementation, this would write to disk
            page->mark_clean();
        }
        
        return true;
    }
    
    PageManager::Stats PageManager::get_stats() const {
        Stats stats;
        stats.total_pages = max_pages_;
        stats.used_pages = current_pages_;
        stats.page_size = page_size_;
        stats.total_memory = current_pages_ * page_size_;
        
        size_t dirty_pages = 0;
        size_t pinned_pages = 0;
        
        for (const auto& [page_id, page] : pages_) {
            if (page->is_dirty()) {
                dirty_pages++;
            }
            if (page->get_ref_count() > 0) {
                pinned_pages++;
            }
        }
        
        stats.dirty_pages = dirty_pages;
        stats.pinned_pages = pinned_pages;
        stats.hit_rate = 1.0;  // Simplified for in-memory implementation
        
        return stats;
    }
    
    void PageManager::clear() {
        pages_.clear();
        current_pages_ = 0;
        next_page_id_ = 1;
    }
    
    void PageManager::set_replacement_policy(std::unique_ptr<ReplacementPolicy> policy) {
        replacement_policy_ = std::move(policy);
    }
    
    bool PageManager::evict_pages(size_t needed_pages) {
        if (needed_pages == 0) {
            return true;
        }
        
        // Get list of evictable pages (not pinned)
        std::vector<PageId> evictable_pages;
        for (const auto& [page_id, page] : pages_) {
            if (page->get_ref_count() == 0) {
                evictable_pages.push_back(page_id);
            }
        }
        
        if (evictable_pages.size() < needed_pages) {
            return false;  // Not enough evictable pages
        }
        
        // Evict pages according to replacement policy
        for (size_t i = 0; i < needed_pages; i++) {
            PageId victim = replacement_policy_->select_victim(evictable_pages);
            if (victim == INVALID_PAGE_ID) {
                return false;
            }
            
            // Flush if dirty before evicting
            flush_page(victim);
            deallocate_page(victim);
            
            // Remove from evictable list
            evictable_pages.erase(
                std::find(evictable_pages.begin(), evictable_pages.end(), victim)
            );
        }
        
        return true;
    }

} // namespace storage
} // namespace minidb
