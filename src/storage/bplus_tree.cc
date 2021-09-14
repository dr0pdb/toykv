#include "bplus_tree.h"

#include <glog/logging.h>

#include "bplus_tree_page_internal.h"
#include "bplus_tree_page_leaf.h"
#include "default_key_comparator.h"

//
// http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap19.htm
//

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
    LOG(INFO) << "BplusTree::Init: Init with root_page_id: " << root_page_id;

    if (root_page_id == INVALID_PAGE_ID) {
        auto root_page_container_status = buffer_manager_->AllocateNewPage();
        if (!root_page_container_status.ok()) {
            LOG(ERROR) << "BplusTree::Init: allocating root page failed";
            return root_page_container_status.status();
        }

        auto root_page_container = root_page_container_status.value();
        auto root_page = reinterpret_cast<BplusTreeLeafPage*>(
            root_page_container->GetData());

        root_page->InitPage(root_page_container->GetPageId(),
                            PageType::PAGE_TYPE_BPLUS_LEAF, INVALID_PAGE_ID);

        auto unpin_status =
            buffer_manager_->UnpinPage(root_page_container->GetPageId(),
                                       /* is_dirty */ true);
        if (!unpin_status.ok()) {
            LOG(ERROR) << "BplusTree::Init: unpinning root page failed";
            return unpin_status;
        }

        LOG(INFO) << "BplusTree::Init: wrote new root page on disk. updating "
                     "root page id to : "
                  << root_page_container->GetPageId();
        // TODO: write a log entry which sets root to this new root page id

        root_page_id_ = root_page_container->GetPageId();
        return absl::OkStatus();
    }

    root_page_id_ = root_page_id;
    return absl::OkStatus();
}

absl::Status BplusTree::Insert(const WriteOptions& options,
                               absl::string_view key,
                               absl::optional<absl::string_view> value) {
    CHECK_NE(root_page_id_, INVALID_PAGE_ID);
    LOG(INFO) << "BplusTree::Insert: start";
    LOG(INFO) << "key: " << key;
    if (value.has_value()) {
        LOG(INFO) << "value: " << value.value();
    }

    auto status_or_root_page_container =
        buffer_manager_->GetPageWithId(root_page_id_);
    if (!status_or_root_page_container.ok()) {
        LOG(ERROR) << "BplusTree::Insert: getting root page failed";
        return status_or_root_page_container.status();
    }

    auto root_page_container = status_or_root_page_container.value();
    CHECK_NOTNULL(root_page_container);

    bool is_full = IsPageFull(root_page_container);
    if (is_full) {
        LOG(INFO) << "BplusTree::Insert: root page is full. creating new root";

        auto status_or_new_root_page_container =
            buffer_manager_->AllocateNewPage();
        if (!status_or_new_root_page_container.ok()) {
            LOG(ERROR) << "BplusTree::Insert: creating new root page failed";
            return status_or_new_root_page_container.status();
        }

        auto new_root_page_container =
            status_or_new_root_page_container.value();
        CHECK_NOTNULL(new_root_page_container);

        auto split_status =
            SplitChild(new_root_page_container, 0, root_page_container);
        if (!split_status.ok()) {
            LOG(ERROR) << "BplusTree::Insert: creating new root page failed";
            return split_status;
        }

        // TODO: update root_page_id_?

        return InsertNonFull(key, value, root_page_container);
    }

    LOG(INFO) << "BplusTree::Insert: root page is non-full";
    return InsertNonFull(key, value, root_page_container);
}

// TODO: Do I need to take a lock on the page container?
// TODO: think about pinning and unpinning of pages.
absl::Status BplusTree::InsertNonFull(absl::string_view key,
                                      absl::optional<absl::string_view> value,
                                      Page* page_container) {
    CHECK_NOTNULL(page_container);

    auto bplus_tree_page =
        reinterpret_cast<BplusTreePage*>(page_container->GetData());
    auto page_type = bplus_tree_page->GetPageType();
    LOG(INFO) << "BplusTree::InsertNonFull: start for page id: "
              << bplus_tree_page->GetPageId()
              << " and page_type: " << page_type;

    if (page_type == PageType::PAGE_TYPE_BPLUS_LEAF) {
        auto leaf_page =
            reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData());

        // find space for the new key value pair
        auto insert_index = leaf_page->count_;
        for (auto idx = std::max((int32_t)0, (int32_t)leaf_page->count_ - 1);
             idx >= 0 &&
             comp_->Compare(key, leaf_page->data_[idx].key.GetStringData()) ==
                 -1;
             idx--) {
            leaf_page->data_[idx + 1] = leaf_page->data_[idx];
            insert_index--;
        }

        LOG(INFO) << "BplusTree::InsertNonFull: inserting at index: "
                  << insert_index;

        leaf_page->data_[insert_index].key.SetStringData(key);
        if (value.has_value()) {
            leaf_page->data_[insert_index].value.SetStringData(value.value());
        } else {
            leaf_page->data_[insert_index].value.EraseStringData();
        }

        leaf_page->count_++;
        return buffer_manager_->UnpinPage(bplus_tree_page->GetPageId(),
                                          /* is_dirty */ true);
    } else if (page_type == PageType::PAGE_TYPE_BPLUS_INTERNAL) {
        auto internal_page =
            reinterpret_cast<BplusTreeInternalPage*>(page_container->GetData());

        auto insert_index = internal_page->count_;
        for (insert_index = internal_page->count_ - 1;
             insert_index >= 0 &&
             comp_->Compare(
                 key, internal_page->keys_[insert_index].GetStringData()) == -1;
             insert_index--) {
        }
        insert_index++;

        auto child_page_id = internal_page->children_[insert_index];
        auto child_page_container_or_status =
            buffer_manager_->GetPageWithId(child_page_id);
        if (!child_page_container_or_status.ok()) {
            // Log it
            return child_page_container_or_status.status();
        }

        auto child_page_container = child_page_container_or_status.value();
        bool is_full = IsPageFull(child_page_container);
        if (is_full) {
            auto split_status =
                SplitChild(page_container, insert_index, child_page_container);
            if (!split_status.ok()) {
                // LOG it
                return split_status;
            }

            if (comp_->Compare(
                    key, internal_page->keys_[insert_index].GetStringData()) >
                1) {
                insert_index++;

                // TODO: need to flush the child_page_container and update it
                // since insert_index has changed.
            }
        }

        return InsertNonFull(key, value, child_page_container);
    } else {
        // LOG it
        return absl::InternalError(
            "BplusTree::InsertNonFull: unknown page type");
    }

    return absl::OkStatus();
}

bool BplusTree::IsPageFull(Page* page_container) {
    auto bplus_tree_page =
        reinterpret_cast<BplusTreePage*>(page_container->GetData());
    auto page_type = bplus_tree_page->GetPageType();

    if (page_type == PageType::PAGE_TYPE_BPLUS_INTERNAL) {
        return reinterpret_cast<BplusTreeInternalPage*>(
                   page_container->GetData())
            ->IsFull();
    }
    return reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData())
        ->IsFull();
}

// TODO: need to hold locks on the container values before reading/writing data.
absl::Status BplusTree::SplitChild(Page* parent_page_container, uint32_t index,
                                   Page* child_page_container) {
    CHECK_NOTNULL(parent_page_container);
    CHECK_NOTNULL(child_page_container);

    auto parent_page = reinterpret_cast<BplusTreeInternalPage*>(
        parent_page_container->GetData());

    auto status_or_second_child_page_container =
        buffer_manager_->AllocateNewPage();
    if (!status_or_second_child_page_container.ok()) {
        return status_or_second_child_page_container.status();
    }

    auto second_child_page_container =
        status_or_second_child_page_container.value();

    bool is_leaf =
        reinterpret_cast<BplusTreePage*>(child_page_container->GetData())
            ->GetPageType() == PageType::PAGE_TYPE_BPLUS_LEAF;
    if (is_leaf) {
        auto child_page = reinterpret_cast<BplusTreeLeafPage*>(
            child_page_container->GetData());
        auto second_child_page = reinterpret_cast<BplusTreeLeafPage*>(
            second_child_page_container->GetData());
        second_child_page->InitPage(second_child_page_container->GetPageId(),
                                    PageType::PAGE_TYPE_BPLUS_LEAF,
                                    parent_page_container->GetPageId());

        auto total_key_count = child_page->GetTotalKeyCount();
        auto start_right_half = total_key_count / 2;

        // move half of keys from child_page to second_child_page
        for (auto idx = start_right_half; idx < total_key_count; idx++) {
            second_child_page->data_[idx - start_right_half] =
                child_page->data_[idx];
        }

        // add second_child_page as child in the parent page
        for (auto idx = parent_page->count_; idx >= index + 1; idx--) {
            parent_page->children_[idx + 1] = parent_page->children_[idx];
        }
        parent_page->children_[index] =
            second_child_page_container->GetPageId();

        // add median key of child_page to parent page. Since the total key
        // count is even for leaf, we use the lower median.
        for (auto idx = parent_page->count_ - 1; idx >= index; idx--) {
            parent_page->keys_[idx + 1] = parent_page->keys_[idx];
        }
        parent_page->keys_[index] = child_page->data_[start_right_half - 1].key;

        child_page->count_ = total_key_count / 2;
        second_child_page->count_ = total_key_count / 2;
        parent_page->count_++;

        auto unpin_second_child_status =
            buffer_manager_->UnpinPage(second_child_page_container->GetPageId(),
                                       /* is_dirty */ true);
        if (!unpin_second_child_status.ok()) {
            return unpin_second_child_status;
        }
    } else {
        auto child_page = reinterpret_cast<BplusTreeInternalPage*>(
            child_page_container->GetData());
        auto second_child_page = reinterpret_cast<BplusTreeInternalPage*>(
            second_child_page_container->GetData());
        second_child_page->InitPage(second_child_page_container->GetPageId(),
                                    PageType::PAGE_TYPE_BPLUS_INTERNAL,
                                    parent_page_container->GetPageId());

        auto total_key_count = second_child_page->GetTotalKeyCount();
        auto start_right_half = total_key_count / 2 + 1;

        // move half of keys from child_page to second_child_page
        for (auto idx = start_right_half; idx < total_key_count; idx++) {
            second_child_page->keys_[idx - start_right_half] =
                child_page->keys_[idx];
        }

        // move half of the child page ids to the second_child_page
        for (auto idx = start_right_half + 1; idx <= total_key_count; idx++) {
            second_child_page->children_[idx - start_right_half - 1] =
                child_page->children_[idx];
        }

        // add second_child_page as child in the parent page
        for (auto idx = parent_page->count_; idx >= index + 1; idx--) {
            parent_page->children_[idx + 1] = parent_page->children_[idx];
        }
        parent_page->children_[index] =
            second_child_page_container->GetPageId();

        // add median key of child_page to parent page
        for (auto idx = parent_page->count_ - 1; idx >= index; idx--) {
            parent_page->keys_[idx + 1] = parent_page->keys_[idx];
        }
        parent_page->keys_[index] = child_page->keys_[start_right_half - 1];

        child_page->count_ = total_key_count / 2;
        second_child_page->count_ = total_key_count / 2;
        parent_page->count_++;

        auto unpin_second_child_status =
            buffer_manager_->UnpinPage(second_child_page_container->GetPageId(),
                                       /* is_dirty */ true);
        if (!unpin_second_child_status.ok()) {
            return unpin_second_child_status;
        }
    }

    return absl::OkStatus();
}

absl::StatusOr<absl::string_view> BplusTree::Get(const ReadOptions& options,
                                                 absl::string_view key) {
    CHECK_NE(root_page_id_, INVALID_PAGE_ID);
    LOG(INFO) << "BplusTree::Get: Init";
    LOG(INFO) << key;

    auto status_or_root_page_container =
        buffer_manager_->GetPageWithId(root_page_id_);
    if (!status_or_root_page_container.ok()) {
        LOG(ERROR) << "BplusTree::Get: getting root page id failed";
        return status_or_root_page_container.status();
    }

    auto root_page_container = status_or_root_page_container.value();
    CHECK_NOTNULL(root_page_container);

    auto res = GetFromPage(key, root_page_container);

    auto s = buffer_manager_->UnpinPage(root_page_id_);
    if (!s.ok()) {
        LOG(ERROR) << "BplusTree::Get: unpinning root page failed";
        return s;
    }

    return res;
}

absl::StatusOr<absl::string_view> BplusTree::GetFromPage(absl::string_view key,
                                                         Page* page_container) {
    CHECK_NOTNULL(page_container);
    LOG(INFO) << "BplusTree::GetFromPage: Init for page id: "
              << page_container->GetPageId();

    page_container->AquireReadLock();

    bool is_leaf = reinterpret_cast<BplusTreePage*>(page_container->GetData())
                       ->GetPageType() == PageType::PAGE_TYPE_BPLUS_LEAF;
    if (is_leaf) {
        LOG(INFO) << "BplusTree::GetFromPage: Reached the leaf node";

        auto leaf_page =
            reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData());

        for (auto idx = (uint32_t)0; idx < leaf_page->count_; idx++) {
            LOG(INFO) << "BplusTree::GetFromPage: query key: " << key
                      << ". stored key: "
                      << leaf_page->data_[idx].key.GetStringData();

            if (comp_->Compare(
                    key, leaf_page->data_[idx].key.GetStringData()) == 0) {
                LOG(INFO) << "BplusTree::GetFromPage: Found the value";

                page_container->ReleaseReadLock();
                return leaf_page->data_[idx].value.GetStringData();
            }
        }

        page_container->ReleaseReadLock();
        return absl::NotFoundError("key not found in b-tree");
    }

    auto internal_page =
        reinterpret_cast<BplusTreeInternalPage*>(page_container->GetData());
    auto idx = (uint32_t)0;
    while (idx < internal_page->count_ &&
           comp_->Compare(key, internal_page->keys_[idx].GetStringData()) > 0) {
        idx++;
    }

    CHECK(idx <= internal_page->count_)
        << "BplusTree::GetFromPage: programming error. idx > "
           "internal_page->count_";

    auto child_page_container_or_status =
        buffer_manager_->GetPageWithId(internal_page->children_[idx]);
    if (!child_page_container_or_status.ok()) {
        LOG(ERROR) << "BplusTree::GetFromPage: error in reading child page "
                      "from buffer pool";

        page_container->ReleaseReadLock();
        return child_page_container_or_status.status();
    }

    auto child_page_container = child_page_container_or_status.value();
    auto res = GetFromPage(key, child_page_container);

    // TODO: This is inefficient. Implement latch crabbing - release parent lock
    // after locking child.
    page_container->ReleaseReadLock();
    auto unpin_status =
        buffer_manager_->UnpinPage(child_page_container->GetPageId());
    if (!unpin_status.ok()) {
        LOG(ERROR) << "BplusTree::Get: unpinning page failed";
        return unpin_status;
    }
    return res;
}

}  // namespace graphchaindb
