#ifndef STORAGE_ROOT_PAGE_H
#define STORAGE_ROOT_PAGE_H

#include "bplus_tree_page.h"
#include "page.h"
#include "src/common/config.h"

namespace graphchaindb {

// The root page containing the database metadata information.
//
// Format (size in bytes):
//
// ---------------------------------------------------
// | PageType (4) | PageId (4) | IndexRootPageId (4) |
// ---------------------------------------------------
//
class RootPage {
   public:
    RootPage() = default;

    RootPage(const RootPage&) = delete;
    RootPage& operator=(const RootPage&) = delete;

    ~RootPage() = default;

    PageType GetPageType() { return page_type_; }

    page_id_t GetPageId() { return page_id_; }

   private:
    PageType page_type_{PAGE_TYPE_ROOT};
    page_id_t page_id_{ROOT_PAGE_ID};
    // root page of the bplus tree index.
    page_id_t index_root_page_id{INVALID_PAGE_ID};
};

}  // namespace graphchaindb

#endif  // STORAGE_ROOT_PAGE_H