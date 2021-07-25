#ifndef STORAGE_BPLUS_TREE_PAGE_LEAF_H
#define STORAGE_BPLUS_TREE_PAGE_LEAF_H

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_LEAF_PAGE_DATA_OFFSET 24

// The leaf page of a B+ tree which stores the actual key value pair.
//
// Format (size in bytes):
// ---------------------------------------------------------
// | Headers (?) | Key 1 + Value 1 | Key 2 + Value 2 | ... |
// ---------------------------------------------------------
//
// Header
// -------------------------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) | Next PageId (4) |
// -------------------------------------------------------------------
//
class BplusTreeLeafPage : public BplusTreePage {
   public:
    BplusTreeLeafPage() = default;

    BplusTreeLeafPage(const BplusTreeLeafPage&) = delete;
    BplusTreeLeafPage& operator=(const BplusTreeLeafPage&) = delete;

    virtual ~BplusTreeLeafPage();

    // init the leaf page
    void InitPage(page_id_t page_id);

   private:
    page_id_t next_page_id_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_LEAF_H