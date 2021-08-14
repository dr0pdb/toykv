#ifndef STORAGE_LOG_MANAGER_H
#define STORAGE_LOG_MANAGER_H

#include <fstream>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "disk_manager.h"
#include "src/common/config.h"

namespace graphchaindb {

// LogManager is responsible for maintaining write ahead log records.
// todo: Thread safety?
class LogManager {
   public:
    explicit LogManager(DiskManager* disk_manager);

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    ~LogManager() = default;

    // Write a log entry for the key value pair
    absl::Status WriteLogEntry(absl::string_view key, absl::string_view value);

   private:
    DiskManager* disk_manager_;
};

}  // namespace graphchaindb

#endif  // STORAGE_LOG_MANAGER_H
