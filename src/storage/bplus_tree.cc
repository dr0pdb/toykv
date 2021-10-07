#include "bplus_tree.h"

#include <glog/logging.h>

#include "bplus_tree_page_internal.h"
#include "bplus_tree_page_leaf.h"
#include "default_key_comparator.h"

//
// http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap19.htm
//
// Although we store key-value pairs only at leaf.
//

namespace graphchaindb {

BplusTree::BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager,
                     LogManager* log_manager)
    : BplusTree(CHECK_NOTNULL(buffer_manager), CHECK_NOTNULL(disk_manager),
                CHECK_NOTNULL(log_manager), new DefaultKeyComparator()) {}

BplusTree::BplusTree(BufferManager* buffer_manager, DiskManager* disk_manager,
                     LogManager* log_manager, KeyComparator* comp)
    : comp_{CHECK_NOTNULL(comp)},
      buffer_manager_{CHECK_NOTNULL(buffer_manager)},
      disk_manager_{CHECK_NOTNULL(disk_manager)},
      log_manager_{CHECK_NOTNULL(log_manager)} {}

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
        root_page_container->AquireExclusiveLock();

        auto root_page = reinterpret_cast<BplusTreeLeafPage*>(
            root_page_container->GetData());

        root_page->InitPage(root_page_container->GetPageId(),
                            PageType::PAGE_TYPE_BPLUS_LEAF, INVALID_PAGE_ID);

        buffer_manager_->UnpinPage(root_page_container,
                                   /* is_dirty */ true);

        auto new_root_id = root_page_container->GetPageId();
        root_page_container->ReleaseExclusiveLock();

        LOG(INFO) << "BplusTree::Init: wrote new root page on disk. updating "
                     "root page id to : "
                  << new_root_id;

        return UpdateRoot(new_root_id);
    }

    root_page_id_ = root_page_id;
    return absl::OkStatus();
}

absl::Status BplusTree::Insert(const WriteOptions& options,
                               absl::string_view key, absl::string_view value) {
    CHECK_NE(root_page_id_, INVALID_PAGE_ID);
    LOG(INFO) << "BplusTree::Insert: start";
    LOG(INFO) << "key: " << key << " value: " << value;

    auto status_or_root_page_container =
        buffer_manager_->GetPageWithId(root_page_id_);
    if (!status_or_root_page_container.ok()) {
        LOG(ERROR) << "BplusTree::Insert: getting root page failed";
        return status_or_root_page_container.status();
    }

    auto root_page_container = status_or_root_page_container.value();
    CHECK_NOTNULL(root_page_container);

    root_page_container->AquireExclusiveLock();

    bool is_full = IsPageFull(root_page_container);
    absl::Status s;
    if (is_full) {
        LOG(INFO) << "BplusTree::Insert: root page is full. creating new root";

        auto status_or_new_root_page_container =
            buffer_manager_->AllocateNewPage();
        if (!status_or_new_root_page_container.ok()) {
            LOG(ERROR) << "BplusTree::Insert: creating new root page failed";

            root_page_container->ReleaseExclusiveLock();
            return status_or_new_root_page_container.status();
        }

        auto new_root_page_container =
            status_or_new_root_page_container.value();
        CHECK_NOTNULL(new_root_page_container);

        new_root_page_container->AquireExclusiveLock();

        auto new_root_page = reinterpret_cast<BplusTreeInternalPage*>(
            new_root_page_container->GetData());
        new_root_page->InitPage(new_root_page_container->GetPageId(),
                                PageType::PAGE_TYPE_BPLUS_INTERNAL,
                                INVALID_PAGE_ID);

        new_root_page->children_[0] = root_page_id_;

        auto split_status =
            SplitChild(new_root_page_container, 0, root_page_container);
        if (!split_status.ok()) {
            LOG(ERROR) << "BplusTree::Insert: creating new root page failed";

            new_root_page_container->ReleaseExclusiveLock();
            root_page_container->ReleaseExclusiveLock();
            return split_status.status();
        }

        s = UpdateRoot(new_root_page_container->GetPageId());
        if (!s.ok()) {
            LOG(ERROR) << "BplusTree::Insert: updating root failed";

            new_root_page_container->ReleaseExclusiveLock();
            root_page_container->ReleaseExclusiveLock();
            return s;
        }

        buffer_manager_->UnpinPage(root_page_container, /* is_dirty */ true);
        root_page_container->ReleaseExclusiveLock();

        s = InsertNonFull(key, value, new_root_page_container);

        buffer_manager_->UnpinPage(new_root_page_container,
                                   /* is_dirty */ true);
        new_root_page_container->ReleaseExclusiveLock();
    } else {
        LOG(INFO) << "BplusTree::Insert: root page is non-full";
        s = InsertNonFull(key, value, root_page_container);

        buffer_manager_->UnpinPage(root_page_container, /* is_dirty */ true);
        root_page_container->ReleaseExclusiveLock();
    }

    return s;
}

absl::Status BplusTree::InsertNonFull(absl::string_view key,
                                      absl::string_view value,
                                      Page* page_container) {
    CHECK_NOTNULL(page_container);

    auto bplus_tree_page =
        reinterpret_cast<BplusTreePage*>(page_container->GetData());
    auto page_type = bplus_tree_page->GetPageType();
    LOG(INFO) << "BplusTree::InsertNonFull: start for page id: "
              << bplus_tree_page->GetPageId()
              << " and page_type: " << page_type;

    if (page_type == PageType::PAGE_TYPE_BPLUS_LEAF) {
        LOG(INFO) << "BplusTree::InsertNonFull: reached the leaf page";

        auto leaf_page =
            reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData());

        int32_t insert_index = 0;
        bool duplicate = false;
        while (insert_index < leaf_page->count_) {
            int comp = comp_->Compare(
                key, leaf_page->data_[insert_index].key.GetStringData());
            if (comp == 0) {
                duplicate = true;
                break;
            } else if (comp == -1) {
                break;
            }

            insert_index++;
        }

        if (!duplicate) {
            for (int32_t idx = std::max(leaf_page->count_ - 1, 0);
                 idx >= insert_index; idx--) {
                leaf_page->data_[idx + 1] = leaf_page->data_[idx];
            }
        }

        LOG(INFO) << "BplusTree::InsertNonFull: inserting at index: "
                  << insert_index;

        leaf_page->data_[insert_index].key.SetStringData(key);
        leaf_page->data_[insert_index].value.SetStringData(value);

        if (!duplicate) {
            leaf_page->count_++;
        }

    } else if (page_type == PageType::PAGE_TYPE_BPLUS_INTERNAL) {
        LOG(INFO) << "BplusTree::InsertNonFull: an internal node";

        auto internal_page =
            reinterpret_cast<BplusTreeInternalPage*>(page_container->GetData());

        int32_t insert_index = 0;
        for (; insert_index < internal_page->count_ &&
               comp_->Compare(
                   key, internal_page->keys_[insert_index].GetStringData()) > 0;
             insert_index++) {
        }

        LOG(INFO)
            << "BplusTree::InsertNonFull: insert index in the internal node is "
            << insert_index;

        auto child_page_id = internal_page->children_[insert_index];
        auto child_page_container_or_status =
            buffer_manager_->GetPageWithId(child_page_id);
        if (!child_page_container_or_status.ok()) {
            LOG(ERROR) << "BplusTree::InsertNonFull: error while getting child "
                          "page from buffer";
            return child_page_container_or_status.status();
        }

        auto child_page_container = child_page_container_or_status.value();
        child_page_container->AquireExclusiveLock();

        bool is_full = IsPageFull(child_page_container);
        if (is_full) {
            auto split_status =
                SplitChild(page_container, insert_index, child_page_container);
            if (!split_status.ok()) {
                LOG(ERROR)
                    << "BplusTree::InsertNonFull: error while splitting child";

                child_page_container->ReleaseExclusiveLock();
                return split_status.status();
            }

            if (comp_->Compare(
                    key, internal_page->keys_[insert_index].GetStringData()) >
                0) {
                insert_index++;

                buffer_manager_->UnpinPage(child_page_container,
                                           /* is_dirty */ true);
                child_page_container->ReleaseExclusiveLock();

                auto updated_child_page_container_or_status =
                    buffer_manager_->GetPageWithId(
                        internal_page->children_[insert_index]);
                if (!updated_child_page_container_or_status.ok()) {
                    LOG(ERROR) << "BplusTree::InsertNonFull: error while "
                                  "changing insert index";
                    return updated_child_page_container_or_status.status();
                }

                child_page_container =
                    updated_child_page_container_or_status.value();
                child_page_container->AquireExclusiveLock();
            }
        }

        auto insert_status = InsertNonFull(key, value, child_page_container);

        buffer_manager_->UnpinPage(child_page_container, /* is_dirty */ true);
        child_page_container->ReleaseExclusiveLock();

        return insert_status;
    } else {
        LOG(ERROR) << "BplusTree::InsertNonFull: unknown page type";
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

absl::StatusOr<page_id_t> BplusTree::SplitChild(Page* parent_page_container,
                                                int32_t index,
                                                Page* child_page_container) {
    CHECK_NOTNULL(parent_page_container);
    CHECK_NOTNULL(child_page_container);

    LOG(INFO) << "BplusTree::SplitChild: Init with parent page id: "
              << parent_page_container->GetPageId() << " index: " << index
              << " and child page id: " << child_page_container->GetPageId();

    auto parent_page = reinterpret_cast<BplusTreeInternalPage*>(
        parent_page_container->GetData());

    auto status_or_second_child_page_container =
        buffer_manager_->AllocateNewPage();
    if (!status_or_second_child_page_container.ok()) {
        LOG(ERROR) << "BplusTree::SplitChild: error while allocating second "
                      "child page";

        return status_or_second_child_page_container.status();
    }

    auto second_child_page_container =
        status_or_second_child_page_container.value();

    second_child_page_container->AquireExclusiveLock();

    auto second_child_page_id = second_child_page_container->GetPageId();
    bool is_leaf =
        reinterpret_cast<BplusTreePage*>(child_page_container->GetData())
            ->GetPageType() == PageType::PAGE_TYPE_BPLUS_LEAF;
    if (is_leaf) {
        LOG(INFO) << "BplusTree::SplitChild: the child node is a leaf";

        auto child_page = reinterpret_cast<BplusTreeLeafPage*>(
            child_page_container->GetData());
        auto second_child_page = reinterpret_cast<BplusTreeLeafPage*>(
            second_child_page_container->GetData());
        second_child_page->InitPage(second_child_page_container->GetPageId(),
                                    PageType::PAGE_TYPE_BPLUS_LEAF,
                                    parent_page_container->GetPageId());

        auto total_key_count = BPLUS_LEAF_KEY_VALUE_SIZE;
        auto start_right_half = total_key_count / 2;

        LOG(INFO) << "BplusTree::SplitChild: Moving half of keys from "
                     "child_page to second_child_page";

        // move half of keys from child_page to second_child_page
        for (auto idx = start_right_half; idx < total_key_count; idx++) {
            second_child_page->data_[idx - start_right_half] =
                child_page->data_[idx];
        }

        LOG(INFO) << "BplusTree::SplitChild: Adding second_child_page as child "
                     "in the parent page";

        // add second_child_page as child in the parent page
        for (int32_t idx = parent_page->count_; idx >= (int32_t)index + 1;
             idx--) {
            parent_page->children_[idx + 1] = parent_page->children_[idx];
        }
        parent_page->children_[index + 1] =
            second_child_page_container->GetPageId();

        // add median key of child_page to parent page. Since the total key
        // count is even for leaf, we use the lower median.
        for (int32_t idx = parent_page->count_ - 1; idx >= (int32_t)index;
             idx--) {
            parent_page->keys_[idx + 1] = parent_page->keys_[idx];
        }
        parent_page->keys_[index] = child_page->data_[start_right_half - 1].key;

        child_page->count_ = total_key_count / 2;
        second_child_page->count_ = total_key_count / 2;
        parent_page->count_++;

        LOG(INFO) << "BplusTree::SplitChild: Updating counts in all the three "
                     "pages. New child_page count: "
                  << child_page->count_
                  << " second_child_page count: " << second_child_page->count_
                  << " parent_page count: " << parent_page->count_;

        child_page->SetParentPageId(parent_page->GetPageId());
        second_child_page->SetParentPageId(parent_page->GetPageId());

        buffer_manager_->UnpinPage(second_child_page_container,
                                   /* is_dirty */ true);
    } else {
        LOG(INFO)
            << "BplusTree::SplitChild: the child node is an internal node";

        auto child_page = reinterpret_cast<BplusTreeInternalPage*>(
            child_page_container->GetData());
        auto second_child_page = reinterpret_cast<BplusTreeInternalPage*>(
            second_child_page_container->GetData());
        second_child_page->InitPage(second_child_page_container->GetPageId(),
                                    PageType::PAGE_TYPE_BPLUS_INTERNAL,
                                    parent_page_container->GetPageId());

        auto total_key_count = BPLUS_INTERNAL_KEY_PAGE_ID_SIZE;
        auto start_right_half = total_key_count / 2 + 1;

        LOG(INFO) << "BplusTree::SplitChild: Moving half of keys from "
                     "child_page to second_child_page";

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

        LOG(INFO) << "BplusTree::SplitChild: Adding second_child_page as child "
                     "in the parent page";

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

        LOG(INFO)
            << "BplusTree::SplitChild: Updating counts in all the three pages";

        child_page->count_ = total_key_count / 2;
        second_child_page->count_ = total_key_count / 2;
        parent_page->count_++;

        buffer_manager_->UnpinPage(second_child_page_container,
                                   /* is_dirty */ true);
    }

    second_child_page_container->ReleaseExclusiveLock();

    return second_child_page_id;
}

absl::Status BplusTree::Delete(const WriteOptions& options,
                               absl::string_view key) {
    CHECK_NE(root_page_id_, INVALID_PAGE_ID);
    LOG(INFO) << "BplusTree::Delete: Init";
    LOG(INFO) << key;

    auto status_or_root_page_container =
        buffer_manager_->GetPageWithId(root_page_id_);
    if (!status_or_root_page_container.ok()) {
        LOG(ERROR) << "BplusTree::Delete: getting root page id failed";
        return status_or_root_page_container.status();
    }

    auto root_page_container = status_or_root_page_container.value();
    CHECK_NOTNULL(root_page_container);

    root_page_container->AquireExclusiveLock();

    auto deletion_status = DeleteFromPage(key, root_page_container);

    buffer_manager_->UnpinPage(root_page_container, /* is_dirty */ true);
    root_page_container->ReleaseExclusiveLock();
    return deletion_status;
}

absl::Status BplusTree::DeleteFromPage(absl::string_view key,
                                       Page* page_container) {
    CHECK_NOTNULL(page_container);
    LOG(INFO) << "BplusTree::DeleteFromPage: Init";
    LOG(INFO) << key;

    auto bplus_tree_page =
        reinterpret_cast<BplusTreePage*>(page_container->GetData());
    auto page_type = bplus_tree_page->GetPageType();
    LOG(INFO) << "BplusTree::DeleteFromPage: start for page id: "
              << bplus_tree_page->GetPageId()
              << " and page_type: " << page_type;

    if (page_type == PageType::PAGE_TYPE_BPLUS_LEAF) {
        LOG(INFO) << "BplusTree::DeleteFromPage: reached the leaf page";

        auto leaf_page =
            reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData());

        int32_t deletion_index = 0;
        bool exists = false;
        while (deletion_index < leaf_page->count_) {
            int comp = comp_->Compare(
                key, leaf_page->data_[deletion_index].key.GetStringData());
            if (comp == 0) {
                exists = true;
                break;
            } else if (comp == -1) {
                break;
            }

            deletion_index++;
        }

        if (!exists) {
            LOG(ERROR) << "BplusTree::DeleteFromPage: key - " << key
                       << " not found in the database";
            return absl::NotFoundError("Key not found in the database");
        }

        LOG(INFO) << "BplusTree::DeleteFromPage: deleting from index: "
                  << deletion_index;

        for (int idx = deletion_index; idx < leaf_page->count_ - 1; idx++) {
            leaf_page->data_[idx] = leaf_page->data_[idx + 1];
        }
        leaf_page->count_--;
    } else if (page_type == PageType::PAGE_TYPE_BPLUS_INTERNAL) {
        LOG(INFO) << "BplusTree::DeleteFromPage: an internal node";

        auto internal_page =
            reinterpret_cast<BplusTreeInternalPage*>(page_container->GetData());

        int32_t deletion_index = 0;
        for (;
             deletion_index < internal_page->count_ &&
             comp_->Compare(
                 key, internal_page->keys_[deletion_index].GetStringData()) > 0;
             deletion_index++) {
        }

        LOG(INFO)
            << "BplusTree::DeleteFromPage: deletion index in the internal "
               "node is "
            << deletion_index;

        auto child_page_id = internal_page->children_[deletion_index];
        auto child_page_container_or_status =
            buffer_manager_->GetPageWithId(child_page_id);
        if (!child_page_container_or_status.ok()) {
            LOG(ERROR)
                << "BplusTree::DeleteFromPage: error while getting child "
                   "page from buffer";
            return child_page_container_or_status.status();
        }

        auto child_page_container = child_page_container_or_status.value();
        child_page_container->AquireExclusiveLock();

        auto deletion_status = DeleteFromPage(key, child_page_container);
        if (!deletion_status.ok()) {
            return deletion_status;
        }

        if (IsPageLessThanHalfFull(child_page_container)) {
            auto borror_or_merge_status = BorrowOrMergeChild(
                page_container, deletion_index, child_page_container);
            if (!borror_or_merge_status.ok()) {
                LOG(ERROR)
                    << "BplusTree::DeleteFromPage: error while borrow_or_merge "
                       "operation";
                return borror_or_merge_status;
            }
        }

        buffer_manager_->UnpinPage(child_page_container, /* is_dirty */ true);
        child_page_container->ReleaseExclusiveLock();
    } else {
        LOG(ERROR) << "BplusTree::DeleteFromPage: unknown page type";
        return absl::InternalError(
            "BplusTree::DeleteFromPage: unknown page type");
    }

    return absl::OkStatus();
}

absl::Status BplusTree::BorrowOrMergeChild(Page* parent_page_container,
                                           int32_t index,
                                           Page* child_page_container) {
    CHECK_NOTNULL(parent_page_container);
    CHECK_NOTNULL(child_page_container);

    LOG(INFO) << "BplusTree::BorrowOrMergeChild: Init with parent page id: "
              << parent_page_container->GetPageId() << " index: " << index
              << " and child page id: " << child_page_container->GetPageId();

    auto parent_page = reinterpret_cast<BplusTreeInternalPage*>(
        parent_page_container->GetData());

    bool is_leaf =
        reinterpret_cast<BplusTreePage*>(child_page_container->GetData())
            ->GetPageType() == PageType::PAGE_TYPE_BPLUS_LEAF;

    Page* sibling_page_container = nullptr;
    if (index > 0) {
        auto status_or_left_sibling_page_container =
            buffer_manager_->GetPageWithId(parent_page->children_[index - 1]);
        if (!status_or_left_sibling_page_container.ok()) {
            return status_or_left_sibling_page_container.status();
        }

        sibling_page_container = status_or_left_sibling_page_container.value();
    } else {
        auto status_or_right_sibling_page_container =
            buffer_manager_->GetPageWithId(parent_page->children_[index + 1]);
        if (!status_or_right_sibling_page_container.ok()) {
            return status_or_right_sibling_page_container.status();
        }

        sibling_page_container = status_or_right_sibling_page_container.value();
    }
    sibling_page_container->AquireExclusiveLock();

    // if sibling is also less than half full, time to merge
    // otherwise borrow
    bool should_merge = IsPageLessThanHalfFull(sibling_page_container);
    if (should_merge) {
    } else {
    }

    buffer_manager_->UnpinPage(sibling_page_container, /* is_dirty */ true);
    sibling_page_container->ReleaseExclusiveLock();
    return absl::OkStatus();
}

absl::Status BplusTree::MergeChild(Page* parent_page, int32_t index,
                                   Page* left_child, Page* right_child,
                                   bool is_leaf) {
    return absl::OkStatus();
}

bool BplusTree::IsPageLessThanHalfFull(Page* page_container) {
    auto bplus_tree_page =
        reinterpret_cast<BplusTreePage*>(page_container->GetData());
    auto page_type = bplus_tree_page->GetPageType();

    if (page_type == PageType::PAGE_TYPE_BPLUS_INTERNAL) {
        return reinterpret_cast<BplusTreeInternalPage*>(
                   page_container->GetData())
            ->IsLessThanHalfFull();
    }
    return reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData())
        ->IsLessThanHalfFull();
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

    buffer_manager_->UnpinPage(root_page_container);
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

        for (auto idx = 0; idx < leaf_page->count_; idx++) {
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
    auto idx = 0;
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
    buffer_manager_->UnpinPage(child_page_container);
    return res;
}

absl::Status BplusTree::UpdateRoot(page_id_t new_root_id) {
    LOG(INFO) << "BplusTree::UpdateRoot: updating root_page_id_ to "
              << new_root_id;

    absl::StatusOr<std::unique_ptr<LogEntry>> sOrLogEntry =
        log_manager_->PrepareLogEntry(INDEX_ROOT_PAGE_ID_KEY,
                                      std::to_string(new_root_id));
    if (!sOrLogEntry.ok() || *sOrLogEntry == nullptr) {
        LOG(ERROR) << "BplusTree::UpdateRoot: unable to update the root of the "
                      "bplus tree";
        return sOrLogEntry.status();
    }

    absl::Status s = log_manager_->WriteLogEntry(*sOrLogEntry);
    if (!s.ok()) {
        LOG(ERROR) << "BplusTree::UpdateRoot: unable to write log for "
                      "updating the root of the bplus tree";
        return s;
    }

    std::unique_lock l(mu_);
    root_page_id_ = new_root_id;

    return absl::OkStatus();
}

void BplusTree::PrintTree() { PrintNode(root_page_id_); }

void BplusTree::PrintNode(page_id_t page_id, std::string indentation) {
    auto page_container = buffer_manager_->GetPageWithId(page_id).value();
    auto page_type = reinterpret_cast<BplusTreePage*>(page_container->GetData())
                         ->GetPageType();

    if (page_type == PageType::PAGE_TYPE_BPLUS_LEAF) {
        auto leaf_page =
            reinterpret_cast<BplusTreeLeafPage*>(page_container->GetData());

        std::cout << indentation << "page_id: " << page_container->GetPageId()
                  << " page type: leaf"
                  << " parent_page_id: " << leaf_page->GetParentPageId()
                  << " count: " << leaf_page->GetCount()
                  << " next_page_id: " << leaf_page->GetNextPageId()
                  << std::endl;

        std::cout << indentation;
        for (int32_t idx = 0; idx < leaf_page->count_; idx++) {
            std::cout << "key: " << leaf_page->data_[idx].key.GetStringData()
                      << " value: "
                      << leaf_page->data_[idx].value.GetStringData()
                      << " ------ ";
        }
        std::cout << std::endl;
    } else {
        auto internal_page =
            reinterpret_cast<BplusTreeInternalPage*>(page_container->GetData());

        std::cout << indentation << "page_id: " << page_container->GetPageId()
                  << ", page type: internal"
                  << ", parent_page_id: " << internal_page->GetParentPageId()
                  << ", count: " << internal_page->GetCount() << std::endl;

        std::string new_indentation = indentation + "------";

        for (int32_t idx = 0; idx < internal_page->count_; idx++) {
            std::cout << new_indentation
                      << "page_id: " << internal_page->children_[idx]
                      << " key: " << internal_page->keys_[idx].GetStringData()
                      << std::endl;

            PrintNode(internal_page->children_[idx], new_indentation);
            std::cout << std::endl;
        }

        std::cout << new_indentation << "page_id: "
                  << internal_page->children_[internal_page->count_]
                  << " key: no key. last child of internal value" << std::endl;
        PrintNode(internal_page->children_[internal_page->count_],
                  new_indentation);
        std::cout << std::endl;
    }
}

}  // namespace graphchaindb
