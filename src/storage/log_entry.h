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

// LogEntry is a single entry in the logs file which denotes an atomic action.
//
// Header Format (size in bytes)
// ----------------------------------------------
// | Entry type (4) | LogEntryId (8) | size (4) |
// ----------------------------------------------
//
// Data format (size in bytes - starts after Header):
//
//
//
class LogEntry {
   public:
    LogEntry(ln_t log_number, absl::string_view key)
        : entry_type_{LogEntryType::LOG_ENTRY_DELETE},
          log_number_{log_number},
          key_size_{static_cast<uint32_t>(key.size())},
          key_{std::string{key.data(), key.size()}} {
        calculateSize();
    }
    LogEntry(ln_t log_number, absl::string_view key, absl::string_view value)
        : entry_type_{LogEntryType::LOG_ENTRY_SET},
          log_number_{log_number},
          key_size_{static_cast<uint32_t>(key.size())},
          value_size_{static_cast<uint32_t>(value.size())},
          key_{std::string{key.data(), key.size()}},
          value_{std::string{value.data(), value.size()}} {
        calculateSize();
    }

    LogEntry(const LogEntry&) = delete;
    LogEntry& operator=(const LogEntry&) = delete;

    ~LogEntry() = default;

    // Get the total size of the log entry
    uint32_t Size() { return size_; }

    // Get log entry type
    LogEntryType GetType() { return entry_type_; }

    // get log number
    ln_t GetLogNumber() { return log_number_; }

    // get the key size
    size_t GetKeySize() { return key_size_; }

    // get key
    absl::string_view GetKey() { return key_; }

    // get the value size
    size_t GetValueSize() { return value_size_; }

    // get the optional value
    absl::optional<absl::string_view> GetValue() { return value_; }

    // Serialize the contents and store in the given data pointer
    void SerializeTo(char* data) {}

   private:
    void calculateSize() {
        size_ = HEADER_SIZE;

        switch (entry_type_) {
            case LOG_ENTRY_SET:
                size_ += 2 * sizeof(uint32_t) + key_size_ + value_size_;
                break;

            case LOG_ENTRY_DELETE:
                size_ += sizeof(uint32_t) + key_size_;
                break;

            default:
                break;
        }
    }

    static const uint32_t HEADER_SIZE = 16;

    LogEntryType entry_type_{LogEntryType::LOG_ENTRY_INVALID};
    ln_t log_number_{0};
    uint32_t size_{0};
    uint32_t key_size_{0};
    uint32_t value_size_{0};
    std::string key_;
    std::string value_;  // could be empty
};

}  // namespace graphchaindb

#endif  // STORAGE_LOG_ENTRY_H
