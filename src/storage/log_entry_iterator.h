#ifndef STORAGE_LOG_ENTRY_ITERATOR_H
#define STORAGE_LOG_ENTRY_ITERATOR_H

#include "disk_manager.h"
#include "iterator.h"
#include "log_entry.h"

namespace graphchaindb {

// LogEntryIterator is an iterator over log entry.
//
// It is not thread safe.
class LogEntryIterator : public Iterator<LogEntry> {
   public:
    LogEntryIterator(DiskManager* disk_manager);
    LogEntryIterator(DiskManager* disk_manager, int offset);

    LogEntryIterator(const LogEntryIterator&) = delete;
    LogEntryIterator& operator=(const LogEntryIterator&) = delete;

    // Returns if the current position of the iterator is valid
    bool IsValid() override;

    // Seek to the first entry of the source
    // Call IsValid() to ensure that the iterator is valid after the seek.
    absl::Status SeekToFirst() override;

    // Seek to the first entry equal or greater than the given item
    // Call IsValid() to ensure that the iterator is valid after the seek.
    absl::Status SeekEqOrGreaterTo(LogEntry* item) override;

    // Move the iterator to the next position.
    // REQUIRES: current position of the iterator must be valid.
    absl::Status Next() override;

    // Get the item at the current position
    // REQUIRES: current position of the iterator must be valid.
    absl::StatusOr<std::unique_ptr<LogEntry>> GetCurrent() override;

   private:
    int offset_;
    DiskManager* disk_manager_{nullptr};
};

}  // namespace graphchaindb

#endif  // STORAGE_LOG_ENTRY_ITERATOR_H