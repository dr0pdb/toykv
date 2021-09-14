#include "src/storage/bplus_tree.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/common/test_values.h"
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

TEST_F(BplusTreeTest, Sequential100InsertGetSucceeds) {
    EXPECT_TRUE(Init().ok());
    auto count = 10;

    for (auto i = 0; i < count; i++) {
        std::string key = "dummy_key_" + std::to_string(i);
        std::string value = "dummy_value_" + std::to_string(i);

        EXPECT_TRUE(bplus_tree->Insert(dummy_write_options, key, value).ok());
    }

    for (auto i = 0; i < count; i++) {
        std::string key = "dummy_key_" + std::to_string(i);
        std::string value = "dummy_value_" + std::to_string(i);

        auto value_or_status = bplus_tree->Get(dummy_read_options, key);
        EXPECT_TRUE(value_or_status.ok());
        EXPECT_EQ(value, value_or_status->data());
    }
}

}  // namespace graphchaindb
