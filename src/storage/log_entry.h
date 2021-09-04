#ifndef STORAGE_LOG_ENTRY_H
#define STORAGE_LOG_ENTRY_H

#include <glog/logging.h>

#include <fstream>
#include <variant>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
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
    uint32_t GetKeySize() { return key_size_; }

    // get key
    absl::string_view GetKey() { return key_; }

    // get the value size
    uint32_t GetValueSize() { return value_size_; }

    // get the optional value
    absl::optional<absl::string_view> GetValue() {
        if (entry_type_ == LOG_ENTRY_SET) {
            return value_;
        }

        return absl::nullopt;
    }

    // Serialize the contents and store in the given data pointer
    void SerializeTo(char* data) {
        memcpy(data, &entry_type_, sizeof(LogEntryType));
        memcpy(data + LOG_NUMBER_OFFSET, &log_number_, sizeof(ln_t));
        memcpy(data + SIZE_OFFSET, &size_, sizeof(uint32_t));

        memcpy(data + HEADER_SIZE, &key_size_, sizeof(uint32_t));
        memcpy(data + HEADER_SIZE + sizeof(uint32_t), key_.c_str(), key_size_);

        if (entry_type_ == LOG_ENTRY_SET) {
            memcpy(data + HEADER_SIZE + sizeof(uint32_t) + key_size_,
                   &value_size_, sizeof(uint32_t));
            memcpy(data + HEADER_SIZE + 2 * sizeof(uint32_t) + key_size_,
                   value_.c_str(), value_size_);
        }
    }

    // Deserialize a log entry from the buffer
    static std::unique_ptr<LogEntry> DeserializeFrom(char* data) {
        LogEntryType entry_type{LogEntryType::LOG_ENTRY_INVALID};
        ln_t log_number;
        uint32_t sz, key_size;

        memcpy(&entry_type, data, sizeof(LogEntryType));
        memcpy(&log_number, data + LOG_NUMBER_OFFSET, sizeof(ln_t));
        memcpy(&sz, data + SIZE_OFFSET, sizeof(uint32_t));

        memcpy(&key_size, data + HEADER_SIZE, sizeof(uint32_t));
        char* key = new char[key_size];
        memcpy(key, data + HEADER_SIZE + sizeof(uint32_t), key_size);
        absl::string_view key_view(key, key_size);

        if (entry_type != LogEntryType::LOG_ENTRY_SET) {
            LOG(INFO) << "LogEntry::DeserializeFrom: entry_type: " << entry_type
                      << " log_number: " << log_number << " sz: " << sz
                      << " key_size: " << key_size;

            return std::make_unique<LogEntry>(log_number, key_view);
        }

        uint32_t value_size;
        memcpy(&value_size, data + HEADER_SIZE + sizeof(uint32_t) + key_size,
               sizeof(uint32_t));
        char* value = new char[value_size];
        memcpy(value, data + HEADER_SIZE + 2 * sizeof(uint32_t) + key_size,
               value_size);
        absl::string_view value_view(value, value_size);

        LOG(INFO) << "LogEntry::DeserializeFrom: entry_type: " << entry_type
                  << " log_number: " << log_number << " sz: " << sz
                  << " key_size: " << key_size << " value_size: " << value_size;

        return std::make_unique<LogEntry>(log_number, key_view, value_view);
    }

    static constexpr uint32_t HEADER_SIZE =
        sizeof(LogEntryType) + sizeof(ln_t) + sizeof(uint32_t);
    static constexpr uint32_t LOG_NUMBER_OFFSET = sizeof(LogEntryType);
    static constexpr uint32_t SIZE_OFFSET = sizeof(LogEntryType) + sizeof(ln_t);

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

    LogEntryType entry_type_{LogEntryType::LOG_ENTRY_INVALID};
    ln_t log_number_{0};
    uint32_t size_{0};
    uint32_t key_size_{0};
    uint32_t value_size_{0};
    std::string key_;
    std::string value_;  // could be empty
};

// Exposed for testing
static constexpr absl::string_view TEST_KEY_1 = "test_key_1";
static constexpr absl::string_view TEST_KEY_2 = "test_key_2";
static constexpr absl::string_view TEST_VALUE_1 = "test_value_1";
static constexpr absl::string_view TEST_VALUE_2 = "test_value_2";

}  // namespace graphchaindb

#endif  // STORAGE_LOG_ENTRY_H
