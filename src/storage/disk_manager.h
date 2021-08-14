#ifndef STORAGE_DISK_MANAGER_H
#define STORAGE_DISK_MANAGER_H

#include <fstream>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/common/config.h"

namespace graphchaindb {

// DiskManager is responsible for reading and writing to db and log files
// todo: Thread safety?
class DiskManager {
   public:
    explicit DiskManager(absl::string_view db_path);

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    ~DiskManager() = default;

    // Load the database from the given db path.
    //
    // MUST be called before any set/get operation on the database.
    //
    // Returns NotFoundError if the files aren't found
    absl::Status LoadDB();

    // Create the db and log files on disk.
    absl::Status CreateDBFilesAndLoadDB();

   private:
    std::string db_path_;
    std::fstream db_file_;
    std::fstream log_file_;
};

}  // namespace graphchaindb

#endif  // STORAGE_DISK_MANAGER_H
