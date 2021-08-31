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

    // Sets the given value corresponding to the given key.
    // overwrites the existing value if it exists.
    absl::Status Set(const WriteOptions& options, absl::string_view key,
                     absl::string_view value);

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<std::string> Get(const ReadOptions& options,
                                    absl::string_view key);

   private:
    // update the btree root page id in the db root page
    absl::Status UpdateRootPageId(page_id_t root_page_id);

    KeyComparator* comp_;
    BufferManager* buffer_manager_;
    DiskManager* disk_manager_;

    absl::Mutex mu_;  // protects root_page_, ...
    page_id_t root_page_id_{INVALID_PAGE_ID} GUARDED_BY(mu_);
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_H