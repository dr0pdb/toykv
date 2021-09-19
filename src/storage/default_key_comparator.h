#ifndef DEFAULT_KEY_COMPARATOR_H
#define DEFAULT_KEY_COMPARATOR_H

#include "absl/strings/string_view.h"
#include "key_comparator.h"

namespace graphchaindb {

// Default implementation of the key comparator that compares lexicographically.
class DefaultKeyComparator : public KeyComparator {
   public:
    DefaultKeyComparator() = default;

    DefaultKeyComparator(const DefaultKeyComparator&) = delete;
    DefaultKeyComparator& operator=(const DefaultKeyComparator&) = delete;

    ~DefaultKeyComparator() override = default;

    // Compare operation compares the two strings
    //
    // Returns
    // -1 if first < second
    // 0 if first == second
    // 1 if first > second
    int Compare(const absl::string_view a, const absl::string_view b) override {
        auto comp = a.compare(b);
        return (comp == 0) ? comp : ((comp < 0) ? -1 : 1);
    }
};

}  // namespace graphchaindb

#endif  // DEFAULT_KEY_COMPARATOR_H