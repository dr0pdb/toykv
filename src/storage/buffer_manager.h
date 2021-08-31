#ifndef STORAGE_BUFFER_MANAGER_H
#define STORAGE_BUFFER_MANAGER_H

#include <map>

#include "absl/status/statusor.h"
#include "src/common/config.h"
#include "src/storage/disk_manager.h"
#include "src/storage/page.h"

namespace graphchaindb {

// BufferManager manages all of the pages in the storage.
//
// Maintains a cache of pages in memory. It retrieves the pages from
// disk and stores them in the cache. It also allocates new pages when
// requested.
//
// TODO: Thread safety?
//
class BufferManager {
   public:
    explicit BufferManager(DiskManager* disk_manager);

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    ~BufferManager() = default;

    // Get the page with the given id and pins it
    absl::StatusOr<Page*> GetPageWithId(page_id_t page_id);

    // Allocate a new page.
    absl::StatusOr<Page*> AllocateNewPage();

    // Unpin the page with the given id
    absl::Status UnpinPage(page_id_t page_id, bool is_dirty = false);

   private:
    // find an empty slot in the cache or evict one of the pages
    absl::StatusOr<int> findIndexToEvict();

    DiskManager* disk_manager_;
    Page cache_[PAGE_BUFFER_SIZE];
    std::map<page_id_t, int> page_id_to_cache_index_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BUFFER_MANAGER_H
