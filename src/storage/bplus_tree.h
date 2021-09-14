#ifndef STORAGE_BPLUS_TREE_H
#define STORAGE_BPLUS_TREE_H

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/mutex.h"
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

    // Insert the given value corresponding to the given key. If the value is
    // not provided, a tombstone value is written which indicates deletion.
    //
    // overwrites the existing value if it exists.
    absl::Status Insert(const WriteOptions& options, absl::string_view key,
                        absl::optional<absl::string_view> value);

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<absl::string_view> Get(const ReadOptions& options,
                                          absl::string_view key);

   private:
    absl::Status InsertNonFull(absl::string_view key,
                               absl::optional<absl::string_view> value,
                               Page* page);

    // Split the given child page into two pages and make it the child of the
    // parent page at the given index (0 based)
    //
    // IMPORTANT: Doesn't unpin the parent_page and child_page
    absl::Status SplitChild(Page* parent_page, uint32_t index,
                            Page* child_page);

    absl::StatusOr<absl::string_view> GetFromPage(absl::string_view key,
                                                  Page* page_container);

    bool IsPageFull(Page* page);

    KeyComparator* comp_;
    BufferManager* buffer_manager_;
    DiskManager* disk_manager_;

    absl::Mutex mu_;  // protects root_page_, ...
    page_id_t root_page_id_{INVALID_PAGE_ID} GUARDED_BY(mu_);
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_H