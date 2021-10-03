#ifndef STORAGE_LOG_MANAGER_H
#define STORAGE_LOG_MANAGER_H

#include <fstream>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "disk_manager.h"
#include "option.h"
#include "src/common/config.h"
#include "src/storage/log_entry.h"
#include "src/storage/log_entry_iterator.h"

namespace graphchaindb {

// LogManager is responsible for maintaining write ahead log records.
// todo: Thread safety?
class LogManager {
   public:
    explicit LogManager(DiskManager* disk_manager);

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    ~LogManager() = default;

    // Get the log entry iterator to iterate the log entries during recovery
    std::unique_ptr<LogEntryIterator> GetLogEntryIterator() {
        return std::make_unique<LogEntryIterator>(disk_manager_);
    }

    // Prepare the log entry.
    // The operation is set if the optional value is present otherwise it is
    // delete.
    absl::StatusOr<std::unique_ptr<LogEntry>> PrepareLogEntry(
        absl::string_view key, absl::optional<absl::string_view> value);

    // Write a log entry for the key value pair
    // TODO: might need to return the log number later
    absl::Status WriteLogEntry(std::unique_ptr<LogEntry>& log_entry);

    // Set the next log number that will be used
    void SetNextLogNumber(ln_t next_ln) { next_ln_ = next_ln; }

   private:
    DiskManager* disk_manager_;
    ln_t next_ln_{INVALID_LOG_NUMBER};
};

}  // namespace graphchaindb

#endif  // STORAGE_LOG_MANAGER_H
