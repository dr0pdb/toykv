#include "default_key_comparator.h"

namespace graphchaindb {

int DefaultKeyComparator::Compare(const absl::string_view a,
                                  const absl::string_view b) {
    return a.compare(b);
}

}  // namespace graphchaindb
