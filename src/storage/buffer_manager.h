#ifndef STORAGE_BUFFER_MANAGER_H
#define STORAGE_BUFFER_MANAGER_H

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
class BufferManager {
   public:
    explicit BufferManager(DiskManager* disk_manager);

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    ~BufferManager() = default;

    // Get the page with the given id.
    absl::StatusOr<Page*> GetPageWithId(page_id_t page_id);

    // Allocate a new page.
    absl::StatusOr<Page*> AllocateNewPage();

    // TODO: Add method for marking page as dirty?

   private:
    DiskManager* disk_manager_;
    Page cache_[PAGE_BUFFER_SIZE];
};

}  // namespace graphchaindb

#endif  // STORAGE_BUFFER_MANAGER_H
