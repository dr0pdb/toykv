#include "buffer_manager.h"

#include <glog/logging.h>

namespace graphchaindb {

BufferManager::BufferManager(DiskManager* disk_manager)
    : disk_manager_{CHECK_NOTNULL(disk_manager)} {}

absl::StatusOr<Page*> BufferManager::GetPageWithId(page_id_t page_id) {
    auto cache_itr = page_id_to_cache_index_.find(page_id);
    if (cache_itr != page_id_to_cache_index_.end()) {
        int cache_index = cache_itr->second;

        cache_[cache_index].AquireExclusiveLock();
        cache_[cache_index].pin_count_++;
        cache_[cache_index].ReleaseExclusiveLock();

        return &cache_[cache_index];
    }

    auto index_or_status = findIndexToEvict();
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

absl::StatusOr<Page*> BufferManager::AllocateNewPage() { return nullptr; }

absl::Status BufferManager::UnpinPage(page_id_t page_id, bool is_dirty) {
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

// TODO: implement this.
// If there is an empty slot, return that
// otherwise if there is an unpinned entry, evict it and return index
// If dirty bit is set, first flush to disk and then evict
absl::StatusOr<int> BufferManager::findIndexToEvict() { return 0; }

}  // namespace graphchaindb
