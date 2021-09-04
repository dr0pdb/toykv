#ifndef STORAGE_ITERATOR_H
#define STORAGE_ITERATOR_H

#include <algorithm>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace graphchaindb {

// Iterator interface for iterating through any iteratable.
template <typename Item>
class Iterator {
   public:
    Iterator() = default;

    Iterator(const Iterator&) = delete;
    Iterator& operator=(const Iterator&) = delete;

    ~Iterator() = default;

    // Returns if the current position of the iterator is valid
    virtual bool IsValid() = 0;

    // Seek to the first entry of the source
    // Call IsValid() to ensure that the iterator is valid after the seek.
    virtual absl::Status SeekToFirst() = 0;

    // Seek to the first entry equal or greater than the given item
    // Call IsValid() to ensure that the iterator is valid after the seek.
    virtual absl::Status SeekEqOrGreaterTo(Item* item) = 0;

    // Move the iterator to the next position.
    // REQUIRES: current position of the iterator must be valid.
    virtual absl::Status Next() = 0;

    // Get the item at the current position
    // REQUIRES: current position of the iterator must be valid.
    // INFO: transfers ownership of the Item if its heap allocated
    virtual absl::StatusOr<std::unique_ptr<Item>> GetCurrent() = 0;
};

}  // namespace graphchaindb

#endif  // STORAGE_ITERATOR_H