#include "buffer_manager.h"

#include <glog/logging.h>

namespace graphchaindb {

BufferManager::BufferManager(DiskManager* disk_manager, LogManager* log_manager)
    : disk_manager_{CHECK_NOTNULL(disk_manager)},
      log_manager_{CHECK_NOTNULL(log_manager)} {}

absl::Status BufferManager::Init(page_id_t next_page_id) {
    LOG(INFO) << "BufferManager::Init: Start with next_page_id "
              << next_page_id;

    next_page_id_ = next_page_id;
    return absl::OkStatus();
}

absl::StatusOr<Page*> BufferManager::GetPageWithId(page_id_t page_id) {
    LOG(INFO) << "BufferManager::GetPageWithId: Start with page_id " << page_id;

    // TODO: needs lock
    auto cache_itr = page_id_to_cache_index_.find(page_id);
    if (cache_itr != page_id_to_cache_index_.end()) {
        int cache_index = cache_itr->second;

        cache_[cache_index].AquireExclusiveLock();
        cache_[cache_index].pin_count_++;
        cache_[cache_index].ReleaseExclusiveLock();

        return &cache_[cache_index];
    }

    auto index_or_status = findIndexToEvict(page_id);
    if (!index_or_status.ok()) {
        // TODO: log
        return index_or_status.status();
    }

    int cache_index = index_or_status.value();
    cache_[cache_index].AquireExclusiveLock();
    cache_[cache_index].pin_count_++;
    cache_[cache_index].ReleaseExclusiveLock();

    return &cache_[cache_index];
}

absl::StatusOr<Page*> BufferManager::AllocateNewPage() {
    LOG(INFO) << "BufferManager::AllocateNewPage: Start";

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

    // todo: needs a lock
    auto page_id = next_page_id_++;

    auto indexOrStatus = findIndexToEvict(page_id);
    if (!indexOrStatus.ok()) {
        // TODO: logs
        return indexOrStatus.status();
    }

    auto index = indexOrStatus.value();

    cache_[index].AquireExclusiveLock();
    cache_[index].pin_count_++;
    cache_[index].page_id_ = page_id;
    cache_[index].is_page_dirty_ = false;
    cache_[index].ReleaseExclusiveLock();

    // TODO: update the maps

    return &cache_[index];
}

absl::Status BufferManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    LOG(INFO) << "BufferManager::UnpinPage: Start with page_id: " << page_id
              << " is_dirty: " << is_dirty;

    auto cache_itr = page_id_to_cache_index_.find(page_id);
    if (cache_itr == page_id_to_cache_index_.end()) {
        LOG(ERROR) << "BufferManager::UnpinPage: error while "
                      "finding page with id: "
                   << page_id;

        return absl::NotFoundError(
            "BufferManager::UnpinPage: unable to find page with id: " +
            page_id);
    }

    int cache_index = cache_itr->second;
    cache_[cache_index].AquireExclusiveLock();

    if (is_dirty) {
        cache_[cache_index].is_page_dirty_ = true;
    }

    cache_[cache_index].pin_count_--;
    CHECK_GE(cache_[cache_index].pin_count_, 0);

    cache_[cache_index].ReleaseExclusiveLock();
    return absl::OkStatus();
}

// Find a free slot in the buffer cache. It doesn't update the maps.
absl::StatusOr<int> BufferManager::findIndexToEvict(page_id_t new_page_id) {
    LOG(INFO) << "BufferManager::findIndexToEvict: Start with new_page_id "
              << new_page_id;

    int cache_index = -1;
    page_id_t existing_page_id = INVALID_PAGE_ID;

    // todo: needs a lock
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
        // TODO: evict an existing page and update existing_page_id
    }

    if (cache_index == -1) {
        return absl::InternalError("");
    }

    Page* page = &cache_[cache_index];
    page->AquireExclusiveLock();

    if (page->GetPageDirty()) {
        // flush to disk at existing_page_id
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

}  // namespace graphchaindb
