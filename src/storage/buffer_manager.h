#ifndef STORAGE_BUFFER_MANAGER_H
#define STORAGE_BUFFER_MANAGER_H

#include <map>
#include <shared_mutex>

#include "absl/status/statusor.h"
#include "src/common/config.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_manager.h"
#include "src/storage/overflow_page.h"
#include "src/storage/page.h"

namespace graphchaindb {

class BufferManager;

void flushRoutine(BufferManager* buffer_manager);

// BufferManager manages all of the pages in the storage.
//
// Maintains a cache of pages in memory. It retrieves the pages from
// disk and stores them in the cache. It also allocates new pages when
// requested.
//
// It is thread safe.
//
class BufferManager {
   public:
    explicit BufferManager(DiskManager* disk_manager, LogManager* log_manager);

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    ~BufferManager() = default;

    // Init the buffer manager after recovery and before any new operation
    absl::Status Init(page_id_t next_page_id);

    // Get the page with the given id and pins it
    absl::StatusOr<Page*> GetPageWithId(page_id_t page_id);

    // Get an overflow page which at least contains the given capacity. The
    // capacity only includes the length of the string. It shouldn't include the
    // space required for storing the length itself.
    absl::StatusOr<Page*> GetOverflowPageWithCapacity(int required_capacity);

    // Allocates a new page and pins it
    absl::StatusOr<Page*> AllocateNewPage();

    // Unpin the given page
    // ASSUMES: Appropriate lock on the page is held by the caller
    void UnpinPage(Page* page, bool is_dirty = false);

    // Routine which is called periodically to flush the unpinned dirty
    // pages to disk
    //
    // Acquires the exclusive lock
    void flushToDisk();

   private:
    // Find an empty slot in the cache or evict one of the pages
    // Also updates the search maps in case a slot was found
    // REQUIRES: mu_ to be held by the caller
    absl::StatusOr<int> findIndexToEvict(page_id_t new_page_id);

    DiskManager* disk_manager_;
    LogManager* log_manager_;
    std::shared_mutex mu_;  // protects page_id_to_cache_index_ and
                            // cache_index_to_page_id_
    page_id_t next_page_id_{STARTING_NORMAL_PAGE_ID};
    std::map<page_id_t, int> page_id_to_cache_index_;
    std::map<int, page_id_t> cache_index_to_page_id_;
    int eviction_start_idx_ = 0;
    std::vector<page_id_t> overflow_pages_;
    Page cache_[PAGE_BUFFER_SIZE];
};

}  // namespace graphchaindb

#endif  // STORAGE_BUFFER_MANAGER_H
