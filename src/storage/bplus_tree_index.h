#ifndef STORAGE_BPLUS_TREE_INDEX_H
#define STORAGE_BPLUS_TREE_INDEX_H

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "bplus_tree.h"
#include "buffer_manager.h"
#include "disk_manager.h"
#include "log_manager.h"
#include "option.h"
#include "src/common/config.h"

namespace graphchaindb {

// A B+ tree based index storing variable length key-value pairs.
//
// It is thread safe.
//
class BplusTreeIndex {
   public:
    BplusTreeIndex(BufferManager* buffer_manager, DiskManager* disk_manager,
                   LogManager* log_manager);
    BplusTreeIndex(BufferManager* buffer_manager, DiskManager* disk_manager,
                   LogManager* log_manager, KeyComparator* comp);

    BplusTreeIndex(const BplusTreeIndex&) = delete;
    BplusTreeIndex& operator=(const BplusTreeIndex&) = delete;

    ~BplusTreeIndex() = default;

    // Init the index
    absl::Status Init(page_id_t root_page_id = INVALID_PAGE_ID);

    // Sets the given value corresponding to the given key.
    // overwrites the existing value if it exists.
    absl::Status Set(const WriteOptions& options, absl::string_view key,
                     absl::string_view value);

    // Deletes the given key value pair from the index.
    absl::Status Delete(const WriteOptions& options, absl::string_view key);

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<std::string> Get(const ReadOptions& options,
                                    absl::string_view key);

   private:
    BufferManager* buffer_manager_;
    DiskManager* disk_manager_;
    LogManager* log_manager_;
    BplusTree* bplus_tree_;
};

}  // namespace graphchaindb

#endif  // STORAGE_BPLUS_TREE_INDEX_H