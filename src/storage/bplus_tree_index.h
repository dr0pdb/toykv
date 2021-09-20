#ifndef STORAGE_BPLUS_TREE_INDEX_H
#define STORAGE_BPLUS_TREE_INDEX_H

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "bplus_tree.h"
#include "buffer_manager.h"
#include "disk_manager.h"
#include "option.h"
#include "src/common/config.h"

namespace graphchaindb {

// A B+ tree based index storing variable length key-value pairs.
//
// TODO: Thread safety?
//
class BplusTreeIndex {
   public:
    BplusTreeIndex(BufferManager* buffer_manager, DiskManager* disk_manager);
    BplusTreeIndex(BufferManager* buffer_manager, DiskManager* disk_manager,
                   KeyComparator* comp);

    BplusTreeIndex(const BplusTreeIndex&) = delete;
    BplusTreeIndex& operator=(const BplusTreeIndex&) = delete;

    ~BplusTreeIndex() = default;

    // Init the index
    absl::Status Init(page_id_t root_page_id = INVALID_PAGE_ID);

    // Sets the given value corresponding to the given key.
    // overwrites the existing value if it exists.
    absl::Status Set(const WriteOptions& options, absl::string_view key,
                     absl::optional<absl::string_view> value);

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<std::string> Get(const ReadOptions& options,
                                    absl::string_view key);

   private:
    BufferManager* buffer_manager_;
    DiskManager* disk_manager_;
    BplusTree* bplus_tree_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_INDEX_H