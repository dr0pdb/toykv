#ifndef STORAGE_DISK_MANAGER_H
#define STORAGE_DISK_MANAGER_H

#include <fstream>

#include "absl/strings/string_view.h"
#include "src/common/config.h"

namespace graphchaindb {

// DiskManager is responsible for reading and writing to db and log files
// todo: Thread safety?
class DiskManager {
   public:
    DiskManager(absl::string_view db_path);

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    ~DiskManager() = default;

   private:
    std::fstream db_file_;
    std::fstream log_file_;
};

}  // namespace graphchaindb

#endif  // STORAGE_DISK_MANAGER_H
