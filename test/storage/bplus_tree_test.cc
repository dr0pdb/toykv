#include "src/storage/bplus_tree.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <map>
#include <memory>
#include <random>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/common/test_values.h"
#include "src/storage/bplus_tree_page_internal.h"
#include "src/storage/bplus_tree_page_leaf.h"
#include "src/storage/buffer_manager.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_entry.h"
#include "src/storage/log_manager.h"
#include "src/storage/option.h"

namespace graphchaindb {

class BplusTreeTest : public ::testing::Test {
   protected:
    BplusTreeTest() {
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".db");
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".log");
        disk_manager = std::make_unique<DiskManager>(TEST_DB_PATH);
        log_manager = std::make_unique<LogManager>(disk_manager.get());
        buffer_manager = std::make_unique<BufferManager>(disk_manager.get(),
                                                         log_manager.get());
        bplus_tree = std::make_unique<BplusTree>(buffer_manager.get(),
                                                 disk_manager.get());
    }

    absl::Status Init() {
        auto s = disk_manager->CreateDBFilesAndLoadDB();
        if (!s.ok()) {
            return s.status();
        }

        auto s2 = log_manager->Init();
        if (!s2.ok()) {
            return s2;
        }

        auto s3 = buffer_manager->Init(STARTING_NORMAL_PAGE_ID);
        if (!s3.ok()) {
            return s3;
        }

        return bplus_tree->Init();
    }

    WriteOptions dummy_write_options;
    ReadOptions dummy_read_options;
    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
    std::unique_ptr<BufferManager> buffer_manager;
    std::unique_ptr<BplusTree> bplus_tree;
};

TEST_F(BplusTreeTest, InitSucceeds) { EXPECT_TRUE(Init().ok()); }

TEST_F(BplusTreeTest, SingleInsertGetSucceeds) {
    EXPECT_TRUE(Init().ok());

    EXPECT_TRUE(
        bplus_tree->Insert(dummy_write_options, TEST_KEY_1, TEST_VALUE_1).ok());

    auto value_or_status = bplus_tree->Get(dummy_read_options, TEST_KEY_1);
    EXPECT_TRUE(value_or_status.ok());
    EXPECT_EQ(TEST_VALUE_1, value_or_status->data());
}

TEST_F(BplusTreeTest, SingleItemMultipleInsertGetSucceeds) {
    EXPECT_TRUE(Init().ok());
    int count = 100;

    for (auto i = 0; i < count; i++) {
        std::string value = "dummy_value_" + std::to_string(i);

        EXPECT_TRUE(
            bplus_tree->Insert(dummy_write_options, TEST_KEY_1, value).ok());
    }

    auto value_or_status = bplus_tree->Get(dummy_read_options, TEST_KEY_1);
    EXPECT_TRUE(value_or_status.ok());
    EXPECT_EQ("dummy_value_" + std::to_string(count - 1),
              value_or_status->data());
}

TEST_F(BplusTreeTest, MultipleSequentialInsertSucceeds) {
    EXPECT_TRUE(Init().ok());
    auto count = 100;

    for (auto i = 0; i < count; i++) {
        std::string key = "dummy_key_" + std::to_string(i);
        std::string value = "dummy_value_" + std::to_string(i);

        EXPECT_TRUE(bplus_tree->Insert(dummy_write_options, key, value).ok());
    }

    bplus_tree->PrintTree();
}

TEST_F(BplusTreeTest, MultipleRandomInsertGetSucceeds) {
    EXPECT_TRUE(Init().ok());
    auto count = 1000;
    std::map<std::string, std::string> kv;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(100, 999);

    for (auto i = 0; i < count; i++) {
        std::string suffix = std::to_string(dist(mt));

        std::string key = "dummy_key_" + suffix;
        std::string value = "dummy_value_" + suffix;

        kv[key] = value;

        EXPECT_TRUE(bplus_tree->Insert(dummy_write_options, key, value).ok());
    }

    for (auto kvp : kv) {
        auto value_or_status = bplus_tree->Get(dummy_read_options, kvp.first);
        EXPECT_TRUE(value_or_status.ok());
        EXPECT_EQ(kvp.second, value_or_status->data());
    }

    bplus_tree->PrintTree();
}

TEST_F(BplusTreeTest, SequentialAllDoubleDigitsInsertGetSucceeds) {
    EXPECT_TRUE(Init().ok());
    auto count = 100;

    for (auto i = 10; i < count; i++) {
        std::string key = "dummy_key_" + std::to_string(i);
        std::string value = "dummy_value_" + std::to_string(i);

        EXPECT_TRUE(bplus_tree->Insert(dummy_write_options, key, value).ok());

        bplus_tree->PrintTree();
    }

    bplus_tree->PrintTree();

    for (auto i = 10; i < count; i++) {
        std::string key = "dummy_key_" + std::to_string(i);
        std::string value = "dummy_value_" + std::to_string(i);

        auto value_or_status = bplus_tree->Get(dummy_read_options, key);
        EXPECT_TRUE(value_or_status.ok());
        EXPECT_EQ(value, value_or_status->data());
    }
}

TEST_F(BplusTreeTest, SplitChildLeafSucceeds) {
    EXPECT_TRUE(Init().ok());

    // setup
    auto parent_page_container = buffer_manager->AllocateNewPage().value();
    auto parent_page = reinterpret_cast<BplusTreeInternalPage*>(
        parent_page_container->GetData());
    parent_page->InitPage(parent_page_container->GetPageId(),
                          PageType::PAGE_TYPE_BPLUS_INTERNAL, INVALID_PAGE_ID);

    auto child_page_container = buffer_manager->AllocateNewPage().value();
    auto child_page =
        reinterpret_cast<BplusTreeLeafPage*>(child_page_container->GetData());
    child_page->InitPage(child_page_container->GetPageId(),
                         PageType::PAGE_TYPE_BPLUS_LEAF, INVALID_PAGE_ID);

    parent_page->children_[0] = child_page->GetPageId();
    for (int idx = 0; idx < BPLUS_LEAF_KEY_VALUE_SIZE; idx++) {
        std::string key = "dummy_key_" + std::to_string(idx);
        std::string value = "dummy_value_" + std::to_string(idx);

        child_page->data_[idx].key.SetStringData(key);
        child_page->data_[idx].value.SetStringData(value);
    }

    uint32_t split_index = 0;
    auto median_idx = BPLUS_LEAF_KEY_VALUE_SIZE / 2 - 1;

    auto statusOrNewChildPageId = bplus_tree->SplitChild(
        parent_page_container, split_index, child_page_container);
    EXPECT_TRUE(statusOrNewChildPageId.ok());
    auto second_child_page_id = statusOrNewChildPageId.value();

    EXPECT_EQ(parent_page->GetCount(), 1);
    EXPECT_EQ(child_page->GetCount(), BPLUS_LEAF_KEY_VALUE_SIZE / 2);

    EXPECT_EQ(parent_page->keys_[split_index].GetStringData(),
              child_page->data_[median_idx].key.GetStringData());
    EXPECT_EQ(parent_page->children_[split_index],
              child_page->GetPageId());  // verification that it doesn't
                                         // overwrite this accidently
    EXPECT_EQ(parent_page->children_[split_index + 1], second_child_page_id);

    auto second_child_page_container =
        buffer_manager->GetPageWithId(second_child_page_id).value();
    auto second_child_page = reinterpret_cast<BplusTreeLeafPage*>(
        second_child_page_container->GetData());
    EXPECT_EQ(second_child_page->GetCount(), BPLUS_LEAF_KEY_VALUE_SIZE / 2);

    for (int idx = BPLUS_LEAF_KEY_VALUE_SIZE / 2;
         idx < BPLUS_LEAF_KEY_VALUE_SIZE; idx++) {
        EXPECT_EQ(child_page->data_[idx].key.GetStringData(),
                  second_child_page->data_[idx - BPLUS_LEAF_KEY_VALUE_SIZE / 2]
                      .key.GetStringData());

        EXPECT_EQ(child_page->data_[idx].value.GetStringData(),
                  second_child_page->data_[idx - BPLUS_LEAF_KEY_VALUE_SIZE / 2]
                      .value.GetStringData());
    }

    EXPECT_EQ(child_page->GetParentPageId(), parent_page->GetPageId());
    EXPECT_EQ(second_child_page->GetParentPageId(), parent_page->GetPageId());
}

}  // namespace graphchaindb
