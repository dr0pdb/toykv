#ifndef STORAGE_BPLUS_TREE_PAGE_INTERNAL_H
#define STORAGE_BPLUS_TREE_PAGE_INTERNAL_H

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_INTERNAL_PAGE_DATA_OFFSET 24

// The internal page of a B+ tree which stores the key and child page ids.
//
// Format (size in bytes):
// ------------------------------------------------------------------
// | Headers (?) | Key 1 | PageId 1(4) | Key 2  | PageId 2(4) | ... |
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

   private:
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_INTERNAL_H