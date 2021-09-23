#ifndef STORAGE_BPLUS_TREE_H
#define STORAGE_BPLUS_TREE_H

#include <gtest/gtest_prod.h>

#include <shared_mutex>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "bplus_tree_page_internal.h"
#include "buffer_manager.h"
#include "disk_manager.h"
#include "key_comparator.h"
#include "src/common/config.h"
#include "src/storage/option.h"

namespace graphchaindb {

// BplusTree which stores the key-value pairs at leaf pages.
//
// Both the keys and values are variable length strings.
// It is thread safe
class BplusTree {
   public:
    BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager);
    BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager,
              KeyComparator* comp);

    BplusTree(const BplusTree&) = delete;
    BplusTree& operator=(const BplusTree&) = delete;

    ~BplusTree();

    // Init the tree. Sets the root page if first_time is true
    // TODO: take the root page id and return statusOr of pageid?
    absl::Status Init(page_id_t root_page_id = INVALID_PAGE_ID);

    // Insert the given value corresponding to the given key.
    //
    // overwrites the existing value if it exists.
    absl::Status Insert(const WriteOptions& options, absl::string_view key,
                        absl::string_view value);

    // Delete the given key and it's corresponding value.
    //
    // Returns NotFoundError if the key is not found.
    absl::Status Delete(const WriteOptions& options, absl::string_view key);

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<absl::string_view> Get(const ReadOptions& options,
                                          absl::string_view key);

    // Print the tree for debugging purposes
    // Only for Debugging. Doesn't lock and handle errors.
    void PrintTree();

   private:
    FRIEND_TEST(BplusTreeTest, SplitChildLeafSucceeds);

    // IMPORTANT: Doesn't unpin the page passed to it
    // ASSUMES: Exclusive lock is held on the page
    absl::Status InsertNonFull(absl::string_view key, absl::string_view value,
                               Page* page);

    // Split the given child page into two pages. The index (0 based) denotes
    // where the child page is in the parents children_ array
    //
    // Returns the newly created child page id
    //
    // IMPORTANT: Doesn't unpin the parent_page and child_page
    // ASSUMES: Exclusive locks are held on the parent and child page by
    // the caller
    absl::StatusOr<page_id_t> SplitChild(Page* parent_page, int32_t index,
                                         Page* child_page);

    // IMPORTANT: Doesn't unpin the page passed to it
    // ASSUMES: Exclusive lock is held on the page
    absl::Status DeleteFromPage(absl::string_view key, Page* page_container);

    // ASSUMES: Shared lock is held on the page container
    absl::StatusOr<absl::string_view> GetFromPage(absl::string_view key,
                                                  Page* page_container);

    // ASSUMES: No locks are held on the page
    bool IsPageFull(Page* page);

    absl::Status UpdateRoot(page_id_t new_root_id);

    // Only for Debugging. Doesn't lock and handle errors.
    void PrintNode(page_id_t page_id, std::string indentation = "");

    KeyComparator* comp_;
    BufferManager* buffer_manager_;
    DiskManager* disk_manager_;

    std::shared_mutex mu_;
    page_id_t root_page_id_{INVALID_PAGE_ID};
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_H