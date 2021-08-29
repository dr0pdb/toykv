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

TEST_F(DiskManagerTest, WriteAndReadDeleteLogEntrySucceeds) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());

    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(1, "test key");

    char* data = new char[log_entry->Size()];
    log_entry->SerializeTo(data);
    EXPECT_TRUE(disk_manager->WriteLogEntry(data, log_entry->Size()).ok());

    auto readData = disk_manager->ReadLogEntry(0);
    EXPECT_TRUE(readData.ok());

    auto read_log_entry = LogEntry::DeserializeFrom(readData.value());

    EXPECT_EQ(log_entry->Size(), read_log_entry->Size());
    EXPECT_EQ(log_entry->GetType(), read_log_entry->GetType());
    EXPECT_EQ(log_entry->GetLogNumber(), read_log_entry->GetLogNumber());
    EXPECT_EQ(log_entry->GetKeySize(), read_log_entry->GetKeySize());
    EXPECT_EQ(log_entry->GetValueSize(), read_log_entry->GetValueSize());

    LOG(INFO) << "DiskManagerTest::WriteAndReadDeleteLogEntrySucceeds: Key: "
              << log_entry->GetKey().data() << " "
              << read_log_entry->GetKey().data();
    EXPECT_EQ(log_entry->GetKey(), read_log_entry->GetKey());

    delete data;
}

TEST_F(DiskManagerTest, WriteAndReadSetLogEntrySucceeds) {
    EXPECT_TRUE(disk_manager->CreateDBFilesAndLoadDB().ok());

    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(1, "test key", "test value");

    char* data = new char[log_entry->Size()];
    log_entry->SerializeTo(data);
    EXPECT_TRUE(disk_manager->WriteLogEntry(data, log_entry->Size()).ok());

    auto readData = disk_manager->ReadLogEntry(0);
    EXPECT_TRUE(readData.ok());

    auto read_log_entry = LogEntry::DeserializeFrom(readData.value());

    EXPECT_EQ(log_entry->Size(), read_log_entry->Size());
    EXPECT_EQ(log_entry->GetType(), read_log_entry->GetType());
    EXPECT_EQ(log_entry->GetLogNumber(), read_log_entry->GetLogNumber());
    EXPECT_EQ(log_entry->GetKeySize(), read_log_entry->GetKeySize());
    EXPECT_EQ(log_entry->GetValueSize(), read_log_entry->GetValueSize());

    LOG(INFO) << "DiskManagerTest::WriteAndReadSetLogEntrySucceeds: Key: "
              << log_entry->GetKey().data() << " "
              << read_log_entry->GetKey().data();
    EXPECT_EQ(log_entry->GetKey(), read_log_entry->GetKey());

    LOG(INFO) << "DiskManagerTest::WriteAndReadSetLogEntrySucceeds: Value: "
              << log_entry->GetValue().value().data() << " "
              << read_log_entry->GetValue().value().data();

    EXPECT_EQ(log_entry->GetValue().value(),
              read_log_entry->GetValue().value());

    delete data;
}

}  // namespace graphchaindb
