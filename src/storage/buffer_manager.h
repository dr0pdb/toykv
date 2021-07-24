#ifndef STORAGE_BUFFER_MANAGER_H
#define STORAGE_BUFFER_MANAGER_H

#include "src/storage/disk_manager.h"

namespace graphchaindb {

class BufferManager {
   public:
    BufferManager(DiskManager* disk_manager);

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    ~BufferManager() = default;

   private:
};

}  // namespace graphchaindb

#endif  // STORAGE_BUFFER_MANAGER_H
