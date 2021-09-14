#include "src/storage/log_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/test_values.h"
#include "src/storage/buffer_manager.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_entry.h"

namespace graphchaindb {

class LogManagerTest : public ::testing::Test {
   protected:
    LogManagerTest() {
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".db");
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".log");
        disk_manager = std::make_unique<DiskManager>(TEST_DB_PATH);
        log_manager = std::make_unique<LogManager>(disk_manager.get());
    }

    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
};

TEST_F(LogManagerTest, PrepareAndWriteLogEntrySetSuccess) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());

    EXPECT_TRUE(log_manager->Init().ok());

    auto log_entry = log_manager->PrepareLogEntry(TEST_KEY_1, TEST_VALUE_1);
    EXPECT_TRUE(log_entry.ok());
    EXPECT_EQ(log_entry.value()->GetKey(), TEST_KEY_1);
    EXPECT_EQ(log_entry.value()->GetValue(), TEST_VALUE_1);

    EXPECT_TRUE(log_manager->WriteLogEntry(log_entry.value()).ok());
}

TEST_F(LogManagerTest, PrepareAndWriteLogEntryDeleteSuccess) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());

    EXPECT_TRUE(log_manager->Init().ok());

    auto log_entry = log_manager->PrepareLogEntry(TEST_KEY_1, absl::nullopt);
    EXPECT_TRUE(log_entry.ok());
    EXPECT_EQ(log_entry.value()->GetKey(), TEST_KEY_1);

    EXPECT_TRUE(log_manager->WriteLogEntry(log_entry.value()).ok());
}

}  // namespace graphchaindb
