#include "buffer_manager.h"

namespace graphchaindb {

BufferManager::BufferManager(DiskManager* disk_manager)
    : disk_manager_{disk_manager} {}

absl::StatusOr<Page*> BufferManager::GetPageWithId(page_id_t page_id) {
    // check in the cache. if found, pin and return it.

    return nullptr;
}

absl::StatusOr<Page*> BufferManager::AllocateNewPage() { return nullptr; }

}  // namespace graphchaindb
