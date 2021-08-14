#ifndef STORAGE_BPLUS_TREE_PAGE_INTERNAL_H
#define STORAGE_BPLUS_TREE_PAGE_INTERNAL_H

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_INTERNAL_PAGE_DATA_OFFSET 24

// A container for holding string key and page id pair.
// Both the key and page id are fixed size.
struct BplusTreeKeyPagePair {
    StringContainer key;
    page_id_t page;
};

// The internal page of a B+ tree which stores the key and child page ids.
//
// Format (size in bytes):
// ------------------------------------------------------------------
// | Headers (12) | Key 1 | PageId 1(4) | Key 2  | PageId 2(4) | ... |
// ------------------------------------------------------------------
//
// Header
// -------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) |
// -------------------------------------------------
//
class BplusTreeInternalPage : public BplusTreePage {
   public:
    BplusTreeInternalPage() = default;

    BplusTreeInternalPage(const BplusTreeInternalPage&) = delete;
    BplusTreeInternalPage& operator=(const BplusTreeInternalPage&) = delete;

    virtual ~BplusTreeInternalPage();

    // init the page
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id) {
        BplusTreePage::InitPage(page_id, page_type, parent_page_id);
    }

   private:
    BplusTreeKeyPagePair
        data_[0];  // array of key and corresponding child page ids
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_INTERNAL_H