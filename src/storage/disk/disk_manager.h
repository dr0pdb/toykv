#ifndef STORAGE_DISK_DISK_MANAGER_H
#define STORAGE_DISK_DISK_MANAGER_H

#include "absl/strings/string_view.h"

namespace graphchaindb {

// DiskManager is responsible for ...
class DiskManager {
   public:
    DiskManager(absl::string_view db_path);

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    ~DiskManager() = default;
};

}  // namespace graphchaindb

#endif  // STORAGE_DISK_DISK_MANAGER_H
