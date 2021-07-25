#ifndef STORAGE_BPLUS_TREE_H
#define STORAGE_BPLUS_TREE_H

#include "absl/synchronization/mutex.h"
#include "src/common/config.h"

namespace graphchaindb {

class BplusTree {
   public:
    BplusTree() = default;

    BplusTree(const BplusTree&) = delete;
    BplusTree& operator=(const BplusTree&) = delete;

    virtual ~BplusTree();

   private:
    page_id_t root_page_id_;
    absl::Mutex mutex_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_H