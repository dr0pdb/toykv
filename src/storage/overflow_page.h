#ifndef STORAGE_OVERFLOW_PAGE_H
#define STORAGE_OVERFLOW_PAGE_H

#include "bplus_tree_page.h"
#include "page.h"
#include "src/common/config.h"

namespace graphchaindb {

// A simple general purpose overflow page which can contain overflown data.
//
// Can be used for bplus tree or string container.
//
// Format (size in bytes):
// --------------------------
// | Headers (16) | Content |
// --------------------------
//
// Header
// -------------------------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) | Next PageId (4) |
// -------------------------------------------------------------------
//
class OverflowPage {
   public:
    OverflowPage() = default;

    OverflowPage(const OverflowPage&) = delete;
    OverflowPage& operator=(const OverflowPage&) = delete;

    ~OverflowPage() = default;

    // init the leaf page
    void InitPage(page_id_t page_id);

   private:
    PageType page_type_;
    page_id_t page_id_;
    page_id_t parent_page_id_;
    page_id_t next_page_id_;
};

}  // namespace graphchaindb

#endif  // STORAGE_OVERFLOW_PAGE_H