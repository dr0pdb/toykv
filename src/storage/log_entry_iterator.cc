#include "log_entry_iterator.h"

namespace graphchaindb {

LogEntryIterator::LogEntryIterator(DiskManager* disk_manager)
    : LogEntryIterator(disk_manager, 0) {}

LogEntryIterator::LogEntryIterator(DiskManager* disk_manager, int offset)
    : offset_{offset}, disk_manager_{CHECK_NOTNULL(disk_manager)} {
    total_size_ = disk_manager_->GetLogFileSize();
    CHECK_GE(total_size_, 0) << "LogEntryIterator::LogEntryIterator: error "
                                "while getting log file size";
}

// Returns if the current position of the iterator is valid
bool LogEntryIterator::IsValid() { return offset_ < total_size_; }

// Seek to the first entry of the source
// Call IsValid() to ensure that the iterator is valid after the seek.
absl::Status LogEntryIterator::SeekToFirst() {
    offset_ = 0;
    return absl::OkStatus();
}

// Seek to the first entry equal or greater than the given item
// Call IsValid() to ensure that the iterator is valid after the seek.
absl::Status LogEntryIterator::SeekEqOrGreaterTo(LogEntry* item) {
    // TODO: implement this
    return absl::OkStatus();
}

// Move the iterator to the next position. The next position could be
// invalid, the caller must verify the validity by calling IsValid().
//
// REQUIRES: current position of the iterator must be valid.
absl::Status LogEntryIterator::Next() {
    auto current_log_entry = GetCurrent();
    if (!current_log_entry.ok()) {
        return current_log_entry.status();
    }

    offset_ += current_log_entry.value()->Size();
    return absl::OkStatus();
}

// Get the item at the current position
// REQUIRES: current position of the iterator must be valid.
// INFO: transfers ownership of the log entry
absl::StatusOr<std::unique_ptr<LogEntry>> LogEntryIterator::GetCurrent() {
    auto dataOrStatus = disk_manager_->ReadLogEntry(offset_);
    if (!dataOrStatus.ok()) {
        return dataOrStatus.status();
    }

    return LogEntry::DeserializeFrom(dataOrStatus.value());
}

}  // namespace graphchaindb
