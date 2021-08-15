#ifndef STORAGE_LOG_ENTRY_H
#define STORAGE_LOG_ENTRY_H

#include <fstream>
#include <variant>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "disk_manager.h"
#include "option.h"
#include "src/common/config.h"

namespace graphchaindb {

// Indicates the type of operation the log entry is for.
// This is stored in the header of each log entry.
enum LogEntryType { LOG_ENTRY_INVALID, LOG_ENTRY_SET, LOG_ENTRY_DELETE };

// Contains log entry fields specific to a set operation
struct SetLogEntryFields {
    uint32_t key_length;
    char* key;
    uint32_t value_length;
    char* value;

    uint32_t Size() { return key_length + value_length + 2 * sizeof(uint32_t); }
};

// Contains log entry fields specific to a delete operation
struct DeleteLogEntryFields {
    uint32_t key_length;
    char* key;

    uint32_t Size() { return key_length + sizeof(uint32_t); }
};

using LogEntryData = std::variant<SetLogEntryFields, DeleteLogEntryFields>;

// LogEntry is a single entry in the logs file which denotes an atomic action.
//
// Header Format (size in bytes)
// ----------------------------------------------
// | Entry type (4) | LogEntryId (8) | size (4) |
// ----------------------------------------------
//
class LogEntry {
   public:
    LogEntry() = default;

    LogEntry(const LogEntry&) = delete;
    LogEntry& operator=(const LogEntry&) = delete;

    ~LogEntry() = default;

    // Get the total size of the log entry
    uint32_t Size() { return size_; }

   private:
    void calculateSize() {
        size_ = HEADER_SIZE;

        switch (entry_type_) {
            case LOG_ENTRY_SET:
                // use CHECK to assert the data
                break;

            case LOG_ENTRY_DELETE:
                // use CHECK to assert the data
                break;

            default:
                break;
        }
    }

    static const uint32_t HEADER_SIZE = 16;

    LogEntryType entry_type_{LogEntryType::LOG_ENTRY_INVALID};
    ln_t log_number_{0};
    uint32_t size_{0};
    LogEntryData data_;
};

}  // namespace graphchaindb

#endif  // STORAGE_LOG_ENTRY_H
