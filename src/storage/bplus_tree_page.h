#ifndef STORAGE_BPLUS_TREE_PAGE_H
#define STORAGE_BPLUS_TREE_PAGE_H

#include "page.h"
#include "src/common/config.h"

namespace graphchaindb {

// A common page interface for the B+ tree.
//
// It contains the common header fields which all the
// B+ tree page types have.
//
// Format (size in bytes)
// -------------------------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) |
// -------------------------------------------------------------------
//
class BplusTreePage {
   public:
    BplusTreePage() = default;

    BplusTreePage(const BplusTreePage&) = delete;
    BplusTreePage& operator=(const BplusTreePage&) = delete;

    ~BplusTreePage() = default;

   private:
    PageType page_type_;
    page_id_t page_id_;
    page_id_t parent_page_id_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_H