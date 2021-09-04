#include "log_entry_iterator.h"

namespace graphchaindb {

LogEntryIterator::LogEntryIterator(DiskManager* disk_manager)
    : LogEntryIterator(disk_manager, 0) {}

LogEntryIterator::LogEntryIterator(DiskManager* disk_manager, int offset)
    : offset_{offset}, disk_manager_{CHECK_NOTNULL(disk_manager)} {}

// Returns if the current position of the iterator is valid
bool LogEntryIterator::IsValid() {
    // TODO: implement this
    return true;
}

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

// Move the iterator to the next position.
// REQUIRES: current position of the iterator must be valid.
absl::Status LogEntryIterator::Next() { return absl::OkStatus(); }

// Get the item at the current position
// REQUIRES: current position of the iterator must be valid.
// INFO: transfers ownership of the log entry
absl::StatusOr<std::unique_ptr<LogEntry>> LogEntryIterator::GetCurrent() {
    auto dataOrStatus = disk_manager_->ReadLogEntry(offset_);
    if (!dataOrStatus.ok()) {
        // log it
        return dataOrStatus.status();
    }

    auto data = dataOrStatus.value();
    std::unique_ptr<LogEntry> log_entry = LogEntry::DeserializeFrom(data);
    return log_entry;
}

}  // namespace graphchaindb
