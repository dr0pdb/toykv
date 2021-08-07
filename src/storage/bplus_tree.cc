#include "bplus_tree.h"

#include "default_key_comparator.h"

namespace graphchaindb {

BplusTree::BplusTree() : BplusTree(new DefaultKeyComparator()) {}

BplusTree::BplusTree(KeyComparator* comp) : comp_{comp} {
    // setup root page
}

BplusTree::~BplusTree() { delete comp_; }

}  // namespace graphchaindb
