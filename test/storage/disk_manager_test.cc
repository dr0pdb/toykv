#include "src/storage/disk_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/storage/log_entry.h"

namespace graphchaindb {

class DiskManagerTest : public ::testing::Test {
   protected:
    DiskManagerTest() {
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".db");
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".log");
        disk_manager = std::make_unique<DiskManager>(TEST_DB_PATH);
    }

    std::unique_ptr<DiskManager> disk_manager;
};

TEST_F(DiskManagerTest, LoadDbReturnsNotFoundOnNonExistentDb) {
    EXPECT_TRUE(absl::IsInternal(disk_manager->LoadDB()));
}

TEST_F(DiskManagerTest, CreateDBFilesSuccessWithNonExistentDB) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());
}

TEST_F(DiskManagerTest, WriteLogEntrySucceeds) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());

    LogEntry* log_entry = new LogEntry(1, "test key");
    char* data = new char[log_entry->Size()];
    log_entry->SerializeTo(data);
    EXPECT_TRUE(disk_manager->WriteLogEntry(data, log_entry->Size()).ok());
}

}  // namespace graphchaindb
