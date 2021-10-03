#include "log_manager.h"

#include <glog/logging.h>

#include <memory>

#include "src/common/config.h"

namespace graphchaindb {
LogManager::LogManager(DiskManager* disk_manager)
    : disk_manager_{CHECK_NOTNULL(disk_manager)} {}

absl::StatusOr<std::unique_ptr<LogEntry>> LogManager::PrepareLogEntry(
    absl::string_view key, absl::optional<absl::string_view> value) {
    LOG(INFO) << "LogManager::PrepareLogEntry: Start";
    VLOG(VERBOSE_EXPENSIVE) << "key: " << key;
    CHECK_NE(next_ln_, INVALID_LOG_NUMBER);

    if (value.has_value()) {
        VLOG(VERBOSE_EXPENSIVE) << "value: " << value.value();
        return std::make_unique<LogEntry>(next_ln_++, key, value.value());
    }

    return std::make_unique<LogEntry>(next_ln_++, key);
}

absl::Status LogManager::WriteLogEntry(std::unique_ptr<LogEntry>& log_entry) {
    LOG(INFO) << "LogManager::WriteLogEntry: Start";
    CHECK_NE(next_ln_, INVALID_LOG_NUMBER);

    char* data = new char[log_entry->Size()];
    log_entry->SerializeTo(data);
    return disk_manager_->WriteLogEntry(data, log_entry->Size());
}

}  // namespace graphchaindb
