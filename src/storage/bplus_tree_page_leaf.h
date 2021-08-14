#ifndef STORAGE_BPLUS_TREE_PAGE_LEAF_H
#define STORAGE_BPLUS_TREE_PAGE_LEAF_H

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_LEAF_PAGE_DATA_OFFSET 24

// A container for holding string key and value pairs.
// Both the key and value are fixed sized.
struct BplusTreeKeyValuePair {
    StringContainer key;
    StringContainer value;
};

// The leaf page of a B+ tree which stores the actual key value pair.
//
// Format (size in bytes):
// ---------------------------------------------------------
// | Headers (16) | Key 1 + Value 1 | Key 2 + Value 2 | ... |
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
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id,
                  page_id_t next_page_id = INVALID_PAGE_ID) {
        BplusTreePage::InitPage(page_id, page_type, parent_page_id);
        next_page_id_ = next_page_id;
    }

    // set the next page id
    void SetNextPage(page_id_t next_page_id);

   private:
    page_id_t next_page_id_;
    BplusTreeKeyValuePair data_[0];  // array of key value pairs
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_LEAF_H