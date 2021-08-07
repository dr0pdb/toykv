#ifndef STORAGE_BPLUS_TREE_H
#define STORAGE_BPLUS_TREE_H

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "key_comparator.h"
#include "src/common/config.h"

namespace graphchaindb {

// BplusTree which stores the key-value pairs at leaf pages.
//
// Both the keys and values are variable length strings.
// It is thread safe
class BplusTree {
   public:
    BplusTree();
    BplusTree(KeyComparator* comp);

    BplusTree(const BplusTree&) = delete;
    BplusTree& operator=(const BplusTree&) = delete;

    ~BplusTree();

   private:
    KeyComparator* comp_;

    absl::Mutex mu_;  // protects root_page_id_, ...
    page_id_t root_page_id_ GUARDED_BY(mu_) = INVALID_PAGE_ID;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_H