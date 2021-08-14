#include "bplus_tree_index.h"

#include "bplus_tree.h"

namespace graphchaindb {

BplusTreeIndex::BplusTreeIndex(BufferManager* buffer_manager)
    : buffer_manager_{buffer_manager} {
    bplus_tree_ = new BplusTree();
}

BplusTreeIndex::BplusTreeIndex(BufferManager* buffer_manager,
                               KeyComparator* comp)
    : buffer_manager_{buffer_manager} {
    bplus_tree_ = new BplusTree(comp);
}

}  // namespace graphchaindb
