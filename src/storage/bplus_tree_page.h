#ifndef STORAGE_BPLUS_TREE_PAGE_H
#define STORAGE_BPLUS_TREE_PAGE_H

#include "page.h"
#include "src/common/config.h"
#include "string_container.h"

namespace graphchaindb {

// A common page interface for the B+ tree.
//
// It contains the common header fields which all the
// B+ tree page types have.
//
// Format (size in bytes)
// -------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) |
// -------------------------------------------------
//
class BplusTreePage {
   public:
    BplusTreePage() = default;

    BplusTreePage(const BplusTreePage&) = delete;
    BplusTreePage& operator=(const BplusTreePage&) = delete;

    ~BplusTreePage() = default;

    // init the page.
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id) {
        page_id_ = page_id;
        page_type_ = page_type;
        parent_page_id_ = parent_page_id;
    }

   private:
    PageType page_type_;
    page_id_t page_id_;
    page_id_t parent_page_id_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_H