#include "log_manager.h"

#include <glog/logging.h>

#include "src/common/config.h"

namespace graphchaindb {
LogManager::LogManager(DiskManager* disk_manager)
    : disk_manager_{CHECK_NOTNULL(disk_manager)} {}

absl::Status LogManager::Init() {
    LOG(INFO) << "LogManager::Init: Start";

    // TODO: Read from log file and init the next log number
    next_ln_ = 0;

    return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<LogEntry>> LogManager::PrepareLogEntry(
    absl::string_view key, absl::optional<absl::string_view> value) {
    LOG(INFO) << "LogManager::PrepareLogEntry: Start";

    // setup headers

    // serialize key to binary

    // serialize value to binary

    return absl::OkStatus();
}

absl::Status LogManager::WriteLogEntry(std::unique_ptr<LogEntry>& log_entry) {
    LOG(INFO) << "LogManager::WriteLogEntry: Start";
    // VLOG(VERBOSE_EXPENSIVE) << "key: " << key << " value: " << value;

    return absl::OkStatus();
}

}  // namespace graphchaindb
