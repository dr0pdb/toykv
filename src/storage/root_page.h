#ifndef STORAGE_ROOT_PAGE_H
#define STORAGE_ROOT_PAGE_H

#include "bplus_tree_page.h"
#include "page.h"
#include "src/common/config.h"

namespace graphchaindb {

// The root page containing the database metadata information.
//
// Format (size in bytes):
// ----------------
// | Headers (?) |
// ----------------
//
// Header
// ----------------------------------------------------
// | PageType (4) | PageId (4) | NextPageId (4) | ... |
// ----------------------------------------------------
//
class RootPage {
   public:
    RootPage() = default;

    RootPage(const RootPage&) = delete;
    RootPage& operator=(const RootPage&) = delete;

    ~RootPage() = default;

    PageType GetPageType() { return page_type_; }

    page_id_t GetPageId() { return page_id_; }

    page_id_t GetNextPageId() { return next_page_id_; }

   private:
    PageType page_type_{PAGE_TYPE_ROOT};
    page_id_t page_id_{ROOT_PAGE_ID};
    // the next page id to allocate in the db file. Not the
    // same as the next page id in other type of pages.
    page_id_t next_page_id_{1};
};

}  // namespace graphchaindb

#endif  // STORAGE_ROOT_PAGE_H