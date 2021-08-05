#ifndef STORAGE_BPLUS_TREE_INDEX_H
#define STORAGE_BPLUS_TREE_INDEX_H

#include "bplus_tree.h"
#include "buffer_manager.h"
#include "src/common/config.h"

namespace graphchaindb {

// A B+ tree based index storing variable length key-value pairs.
//
// TODO: Thread safety?
//
class BplusTreeIndex {
   public:
    BplusTreeIndex(BufferManager* buffer_manager);

    BplusTreeIndex(const BplusTreeIndex&) = delete;
    BplusTreeIndex& operator=(const BplusTreeIndex&) = delete;

    ~BplusTreeIndex() = default;

   private:
    BufferManager* buffer_manager_;
    BplusTree* bplus_tree_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_INDEX_H