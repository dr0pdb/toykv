#include "bplus_tree_index.h"

#include <glog/logging.h>

#include "bplus_tree.h"

namespace graphchaindb {

BplusTreeIndex::BplusTreeIndex(BufferManager* buffer_manager,
                               DiskManager* disk_manager)
    : buffer_manager_{CHECK_NOTNULL(buffer_manager)},
      disk_manager_{CHECK_NOTNULL(disk_manager)} {
    bplus_tree_ = new BplusTree(buffer_manager_, disk_manager);
}

BplusTreeIndex::BplusTreeIndex(BufferManager* buffer_manager,
                               DiskManager* disk_manager, KeyComparator* comp)
    : buffer_manager_{CHECK_NOTNULL(buffer_manager)},
      disk_manager_{CHECK_NOTNULL(disk_manager)} {
    CHECK_NOTNULL(comp);
    bplus_tree_ = new BplusTree(buffer_manager_, disk_manager, comp);
}

absl::Status BplusTreeIndex::Init(page_id_t root_page_id) {
    return bplus_tree_->Init(root_page_id);
}

absl::Status BplusTreeIndex::Set(const WriteOptions& options,
                                 absl::string_view key,
                                 absl::string_view value) {
    return absl::OkStatus();
}

absl::StatusOr<std::string> BplusTreeIndex::Get(const ReadOptions& options,
                                                absl::string_view key) {
    return absl::OkStatus();
}

}  // namespace graphchaindb
