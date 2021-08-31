#include "bplus_tree.h"

#include <glog/logging.h>

#include "bplus_tree_page_internal.h"
#include "default_key_comparator.h"

namespace graphchaindb {

BplusTree::BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager)
    : BplusTree(CHECK_NOTNULL(buffer_manager), CHECK_NOTNULL(disk_manager),
                new DefaultKeyComparator()) {}

BplusTree::BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager,
                     KeyComparator* comp)
    : comp_{CHECK_NOTNULL(comp)},
      buffer_manager_{CHECK_NOTNULL(buffer_manager)},
      disk_manager_{CHECK_NOTNULL(disk_manager)} {}

BplusTree::~BplusTree() { delete comp_; }

absl::Status BplusTree::Init(page_id_t root_page_id) {
    if (root_page_id == INVALID_PAGE_ID) {
        auto root_page_container_status = buffer_manager_->AllocateNewPage();
        if (!root_page_container_status.ok()) {
            return root_page_container_status.status();
        }

        auto root_page_container = root_page_container_status.value();
        auto root_page = reinterpret_cast<BplusTreeInternalPage*>(
            root_page_container->GetData());

        root_page->InitPage(root_page_container->PageId(),
                            PageType::PAGE_TYPE_BPLUS_INTERNAL,
                            INVALID_PAGE_ID);

        return absl::OkStatus();
    }

    return absl::OkStatus();
}

absl::Status BplusTree::Set(const WriteOptions& options, absl::string_view key,
                            absl::string_view value) {
    return absl::OkStatus();
}

absl::StatusOr<std::string> BplusTree::Get(const ReadOptions& options,
                                           absl::string_view key) {
    return absl::OkStatus();
}

absl::Status BplusTree::UpdateRootPageId(page_id_t root_page_id) {
    return absl::OkStatus();
}

}  // namespace graphchaindb
