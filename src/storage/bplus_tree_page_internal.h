#ifndef STORAGE_BPLUS_TREE_PAGE_INTERNAL_H
#define STORAGE_BPLUS_TREE_PAGE_INTERNAL_H

#include <gtest/gtest_prod.h>

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_INTERNAL_PAGE_DATA_OFFSET 24

// The internal page of a B+ tree which stores the key and child page ids.
//
// Format (size in bytes):
// -----------------------------------------------
// | Headers (16) | PageId (4) | Key 1 (64) | .. |
// -----------------------------------------------
//
// Header
// -------------------------------------------------------------
// | PageType (4) | PageId (4) | Parent PageId (4) | Count (4) |
// -------------------------------------------------------------
//
class BplusTreeInternalPage : public BplusTreePage {
    friend class BplusTree;

   public:
    BplusTreeInternalPage() = default;

    BplusTreeInternalPage(const BplusTreeInternalPage&) = delete;
    BplusTreeInternalPage& operator=(const BplusTreeInternalPage&) = delete;

    ~BplusTreeInternalPage() = default;

    // init the page
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id) {
        BplusTreePage::InitPage(page_id, page_type, parent_page_id, 0);

        // TODO: set all children to invalid page id
    }

    // Returns if the page is full
    bool IsFull() {
        return BplusTreePage::GetCount() + 1 == BPLUS_INTERNAL_KEY_PAGE_ID_SIZE;
    }

    // Returns if the page is less than half full
    bool IsLessThanHalfFull() {
        // TODO: implement
        return BplusTreePage::GetCount() + 1 == BPLUS_INTERNAL_KEY_PAGE_ID_SIZE;
    }

   private:
    FRIEND_TEST(BplusTreeTest, SplitChildLeafSucceeds);

    StringContainer keys_[BPLUS_INTERNAL_KEY_PAGE_ID_SIZE];
    page_id_t children_[BPLUS_INTERNAL_KEY_PAGE_ID_SIZE + 1];
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_INTERNAL_H