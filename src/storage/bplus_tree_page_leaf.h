#ifndef STORAGE_BPLUS_TREE_PAGE_LEAF_H
#define STORAGE_BPLUS_TREE_PAGE_LEAF_H

#include <glog/logging.h>
#include <gtest/gtest_prod.h>

#include "bplus_tree_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: update this offset once the header format is decided.
#define BPLUS_TREE_LEAF_PAGE_DATA_OFFSET 24

// A container for holding string key and value pairs.
// Both the key and value are fixed sized.
//
// Total size: 128 bytes
struct BplusTreeKeyValuePair {
    StringContainer key;
    StringContainer value;
};

// The leaf page of a B+ tree which stores the actual key value pair.
//
// Format (size in bytes):
// ----------------------------------------------
// | Headers (20) | Key 1 + Value 1 (128) | ... |
// ----------------------------------------------
//
// Header
// --------------------------------------------------------------------------
// | PageType(4) | PageId(4) | Parent PageId(4) | Count(4) | Next PageId(4) |
// --------------------------------------------------------------------------
//
class BplusTreeLeafPage : public BplusTreePage {
    friend class BplusTree;

   public:
    BplusTreeLeafPage() = default;

    BplusTreeLeafPage(const BplusTreeLeafPage&) = delete;
    BplusTreeLeafPage& operator=(const BplusTreeLeafPage&) = delete;

    ~BplusTreeLeafPage() = default;

    // init the leaf page
    //
    // MUST be called after allocating the page and before doing anything useful
    void InitPage(page_id_t page_id, PageType page_type,
                  page_id_t parent_page_id,
                  page_id_t next_page_id = INVALID_PAGE_ID) {
        BplusTreePage::InitPage(page_id, page_type, parent_page_id, 0);
        next_page_id_ = next_page_id;
    }

    // set the next page id
    void SetNextPageId(page_id_t next_page_id);

    page_id_t GetNextPageId() { return next_page_id_; }

    // Returns if the page is full
    bool IsFull() {
        return BplusTreePage::GetCount() == BPLUS_LEAF_KEY_VALUE_SIZE;
    }

    // Returns if the page is less than half full
    bool IsLessThanHalfFull() {
        // TODO: implement
        return BplusTreePage::GetCount() == BPLUS_LEAF_KEY_VALUE_SIZE;
    }

   private:
    FRIEND_TEST(BplusTreeTest, SplitChildLeafSucceeds);

    page_id_t next_page_id_;
    BplusTreeKeyValuePair
        data_[BPLUS_LEAF_KEY_VALUE_SIZE];  // array of key value pairs
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_PAGE_LEAF_H