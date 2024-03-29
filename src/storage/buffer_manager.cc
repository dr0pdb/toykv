#include "buffer_manager.h"

#include <glog/logging.h>

#include <chrono>
#include <thread>

namespace graphchaindb {

BufferManager::BufferManager(DiskManager* disk_manager, LogManager* log_manager)
    : disk_manager_{CHECK_NOTNULL(disk_manager)},
      log_manager_{CHECK_NOTNULL(log_manager)} {}

absl::Status BufferManager::Init(page_id_t next_page_id) {
    LOG(INFO) << "BufferManager::Init: Start with next_page_id "
              << next_page_id;

    next_page_id_ = next_page_id;

    std::thread background_flusher(flushRoutine, this);
    background_flusher.detach();

    return absl::OkStatus();
}

absl::StatusOr<Page*> BufferManager::GetPageWithId(page_id_t page_id) {
    LOG(INFO) << "BufferManager::GetPageWithId: Start with page_id " << page_id;

    std::unique_lock l(mu_);

    auto cache_itr = page_id_to_cache_index_.find(page_id);
    if (cache_itr != page_id_to_cache_index_.end()) {
        int cache_index = cache_itr->second;

        LOG(INFO)
            << "BufferManager::GetPageWithId: Found page in cache at index: "
            << cache_index;

        cache_[cache_index].AquireExclusiveLock();
        cache_[cache_index].pin_count_++;
        cache_[cache_index].ReleaseExclusiveLock();

        return &cache_[cache_index];
    }

    LOG(INFO) << "BufferManager::GetPageWithId: Page not found in cache";

    auto index_or_status = findIndexToEvict(page_id);
    if (!index_or_status.ok()) {
        LOG(ERROR) << "BufferManager::GetPageWithId: error while finding index "
                      "to evict";
        return index_or_status.status();
    }

    int cache_index = index_or_status.value();
    auto page = &cache_[cache_index];
    page->AquireExclusiveLock();

    page->page_id_ = page_id;
    disk_manager_->ReadPage(page_id, page->GetData());
    page->pin_count_++;
    page->is_page_dirty_ = false;

    page->ReleaseExclusiveLock();

    return &cache_[cache_index];
}

absl::StatusOr<Page*> BufferManager::GetOverflowPageWithCapacity(
    int required_capacity) {
    LOG(INFO) << "BufferManager::GetOverflowPageWithCapacity: Start with "
                 "required_capacity: "
              << required_capacity;

    Page* result = nullptr;
    for (auto page_id : overflow_pages_) {
        auto page_status = GetPageWithId(page_id);
        if (!page_status.ok()) {
        }

        auto page = page_status.value();
        auto overflow_page = reinterpret_cast<OverflowPage*>(page->GetData());
        if (overflow_page->RemainingCapacity() >= required_capacity + 4) {
            result = page;
            break;
        }

        UnpinPage(page, false);
    }

    if (result == nullptr) {
        auto page_status = AllocateNewPage();
        if (!page_status.ok()) {
        }

        auto page = page_status.value();
        auto overflow_page = reinterpret_cast<OverflowPage*>(page->GetData());
        overflow_page->InitPage(page->GetPageId());
        disk_manager_->WritePage(page->GetPageId(),
                                 reinterpret_cast<char*>(overflow_page),
                                 /* flush */ true);

        overflow_pages_.emplace_back(page->GetPageId());
        result = page;
    }

    return result;
}

absl::StatusOr<Page*> BufferManager::AllocateNewPage() {
    LOG(INFO) << "BufferManager::AllocateNewPage: Start";

    std::unique_lock l(mu_);

    absl::StatusOr<std::unique_ptr<LogEntry>> sOrLogEntry =
        log_manager_->PrepareLogEntry(NEXT_PAGE_ID_KEY,
                                      std::to_string(next_page_id_ + 1));
    if (!sOrLogEntry.ok() || *sOrLogEntry == nullptr) {
        LOG(ERROR) << "BufferManager::AllocateNewPage: unable to prepare log "
                      "entry for updating next page id";
        return sOrLogEntry.status();
    }

    absl::Status s = log_manager_->WriteLogEntry(*sOrLogEntry);
    if (!s.ok()) {
        LOG(ERROR) << "BufferManager::AllocateNewPage: unable to write log for "
                      "updating next page id"
                      "entry for set operation";
        return s;
    }

    LOG(INFO) << "BufferManager::AllocateNewPage: wrote the log entry";
    auto page_id = next_page_id_++;

    auto indexOrStatus = findIndexToEvict(page_id);
    if (!indexOrStatus.ok()) {
        LOG(ERROR) << "BufferManager::AllocateNewPage: error while finding "
                      "index to evict";
        return indexOrStatus.status();
    }

    auto index = indexOrStatus.value();
    LOG(INFO) << "BufferManager::AllocateNewPage: found slot index: " << index;

    cache_[index].AquireExclusiveLock();
    cache_[index].pin_count_++;
    cache_[index].page_id_ = page_id;
    cache_[index].is_page_dirty_ = false;
    cache_[index].ReleaseExclusiveLock();

    return &cache_[index];
}

void BufferManager::UnpinPage(Page* page, bool is_dirty) {
    LOG(INFO) << "BufferManager::UnpinPage: Start with page_id: "
              << page->GetPageId() << " is_dirty: " << is_dirty;

    if (is_dirty) {
        page->is_page_dirty_ = true;
    }

    // TODO: what if only read lock is held. Is this safe?
    page->pin_count_--;
    CHECK_GE(page->pin_count_, 0);
}

// Find a free slot in the buffer cache. It doesn't update the maps.
// REQUIRES: mu_ to be held by the caller
absl::StatusOr<int> BufferManager::findIndexToEvict(page_id_t new_page_id) {
    LOG(INFO) << "BufferManager::findIndexToEvict: Start with new_page_id "
              << new_page_id;

    int cache_index = -1;
    page_id_t existing_page_id = INVALID_PAGE_ID;
    bool eviction = false;

    if (cache_index_to_page_id_.size() < PAGE_BUFFER_SIZE) {
        for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
            if (cache_index_to_page_id_.find(i) ==
                cache_index_to_page_id_.end()) {
                cache_index = i;
                break;
            }
        }

        CHECK_GE(cache_index, 0)
            << "BufferManager::findIndexToEvict: programming "
               "error - expected index to be non-negative";
    } else {
        int eviction_end_idx =
            (eviction_start_idx_ + PAGE_BUFFER_SIZE - 1) % PAGE_BUFFER_SIZE;

        LOG(INFO) << "BufferManager::findIndexToEvict: starting eviction idx "
                     "search from "
                  << eviction_start_idx_;

        for (; eviction_start_idx_ != eviction_end_idx;
             eviction_start_idx_ =
                 (eviction_start_idx_ + 1) % PAGE_BUFFER_SIZE) {
            LOG(INFO)
                << "BufferManager::findIndexToEvict: current eviction idx "
                   "search from "
                << eviction_start_idx_;

            // it's not possible to increase the pin count concurrently while
            // doing this since we hold the global lock. So this is thread safe
            if (cache_[eviction_start_idx_].pin_count_ == 0) {
                if (cache_[eviction_start_idx_].second_chance_) {
                    cache_[eviction_start_idx_].second_chance_ = false;
                    continue;
                }

                cache_index = eviction_start_idx_;
                eviction = true;
            }
        }
    }

    if (cache_index == -1) {
        LOG(ERROR)
            << "BufferManager::findIndexToEvict: couldn't find a page to evict";
        return absl::InternalError(
            "BufferManager::findIndexToEvict: couldn't find a page to evict");
    }

    LOG(INFO) << "BufferManager::findIndexToEvict: found index to evict: "
              << cache_index;

    Page* page = &cache_[cache_index];
    page->AquireExclusiveLock();
    if (eviction) {
        existing_page_id = page->GetPageId();
    }

    // this is safe to do because the flusher routine won't be flushing since
    // we're holding an exclusive lock.
    if (page->GetPageDirty()) {
        CHECK_NE(existing_page_id, INVALID_PAGE_ID)
            << "BufferManager::findIndexToEvict: programming "
               "error - existing page id is invalid but page is dirty.";

        auto existing_page_write_status =
            disk_manager_->WritePage(existing_page_id, page->GetData());
        if (!existing_page_write_status.ok()) {
            LOG(ERROR) << "BufferManager::findIndexToEvict: error while "
                          "writing existing page to disk";

            page->ReleaseExclusiveLock();
            return existing_page_write_status;
        }
    }

    page->pin_count_ = 0;
    page->ZeroOut();

    page->ReleaseExclusiveLock();

    // update map
    if (existing_page_id != INVALID_PAGE_ID) {
        page_id_to_cache_index_.erase(existing_page_id);
    }
    page_id_to_cache_index_[new_page_id] = cache_index;
    cache_index_to_page_id_[cache_index] = new_page_id;

    return cache_index;
}

void flushRoutine(BufferManager* buffer_manager) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(FLUSH_WAIT_INTERVAL_MILLISECONDS));

    while (true) {
        buffer_manager->flushToDisk();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(FLUSH_WAIT_INTERVAL_MILLISECONDS));
    }
}

void BufferManager::flushToDisk() {
    LOG(INFO) << "BufferManager::flushToDisk: Start";
    std::unique_lock l(mu_);
    std::vector<int> to_flush;

    // first check if we even need to flush any page
    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page = &cache_[i];
        page->AquireReadLock();

        if (page->pin_count_ == 0 && page->is_page_dirty_) {
            to_flush.push_back(i);
        }

        page->ReleaseReadLock();
    }

    LOG(INFO) << "BufferManager::flushToDisk: Number of pages to flush: "
              << to_flush.size();

    for (std::size_t i = 0; i < to_flush.size(); i++) {
        int idx = to_flush[i];
        auto page = &cache_[idx];
        page->AquireExclusiveLock();

        // in between the reading and writing, the pages pin count could have
        // been updated.
        if (page->pin_count_ == 0) {
            auto flush_status =
                disk_manager_->WritePage(page->GetPageId(), page->GetData(),
                                         /* flush */ i + 1 == to_flush.size());
            if (!flush_status.ok()) {
                LOG(ERROR) << "BufferManager::flushToDisk: error while "
                              "flushing the page "
                           << page->GetPageId();
                page->ReleaseExclusiveLock();
                return;
            }
        }

        page->ReleaseExclusiveLock();
    }
}

}  // namespace graphchaindb
