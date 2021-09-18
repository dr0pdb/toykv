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
// -------------------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) | Count (4) |
// -------------------------------------------------------------
//
class BplusTreePage {
    friend class BplusTree;

   public:
    BplusTreePage() = default;

    BplusTreePage(const BplusTreePage&) = delete;
    BplusTreePage& operator=(const BplusTreePage&) = delete;

    ~BplusTreePage() = default;

    // init the page.
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id, uint32_t count) {
        page_id_ = page_id;
        page_type_ = page_type;
        parent_page_id_ = parent_page_id;
        count_ = count;
    }

    // Get the page type
    PageType GetPageType() { return page_type_; }

    // Get the page id
    page_id_t GetPageId() { return page_id_; }

    // Get the parent page id
    page_id_t GetParentPageId() { return parent_page_id_; }

    // Set the parent page id
    void SetParentPageId(page_id_t parent_page_id) {
        parent_page_id_ = parent_page_id;
    }

    // Get the count of keys written in the page
    uint32_t GetCount() { return count_; }

   private:
    PageType page_type_;
    page_id_t page_id_;
    page_id_t parent_page_id_;
    uint32_t count_{0};
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_H