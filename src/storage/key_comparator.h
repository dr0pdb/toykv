#ifndef KEY_COMPARATOR_H
#define KEY_COMPARATOR_H

#include "absl/strings/string_view.h"

namespace graphchaindb {

// An interface for the key comparator
class KeyComparator {
   public:
    KeyComparator() = default;

    KeyComparator(const KeyComparator&) = delete;
    KeyComparator& operator=(const KeyComparator&) = delete;

    ~KeyComparator() = default;

    // Compare operation compares the two strings
    //
    // Returns
    // -1 if first < second
    // 0 if first == second
    // 1 if first > second
    virtual int Compare(const absl::string_view, const absl::string_view) = 0;
};

}  // namespace graphchaindb

#endif  // KEY_COMPARATOR_H