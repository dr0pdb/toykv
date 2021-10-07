#include "src/storage/recovery_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/storage/bplus_tree_index.h"
#include "src/storage/buffer_manager.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_entry.h"
#include "src/storage/log_manager.h"

namespace graphchaindb {

class RecoveryManagerTest : public ::testing::Test {
   protected:
    RecoveryManagerTest() {
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".db");
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".log");
        disk_manager = std::make_unique<DiskManager>(TEST_DB_PATH);
        log_manager = std::make_unique<LogManager>(disk_manager.get());
        buffer_manager = std::make_unique<BufferManager>(disk_manager.get(),
                                                         log_manager.get());
        index = std::make_unique<BplusTreeIndex>(
            buffer_manager.get(), disk_manager.get(), log_manager.get());
        recovery_manager =
            std::make_unique<RecoveryManager>(log_manager.get(), index.get());
    }

    absl::Status Init() {
        auto s = disk_manager->CreateDBFilesAndLoadDB();
        if (!s.ok()) {
            return s.status();
        }

        log_manager->SetNextLogNumber(STARTING_LOG_NUMBER);

        return buffer_manager->Init(STARTING_NORMAL_PAGE_ID);
    }

    absl::Status InsertDummyData() {
        auto entry1 =
            log_manager->PrepareLogEntry(NEXT_PAGE_ID_KEY, std::to_string(10))
                .value();
        log_manager->WriteLogEntry(entry1);

        auto entry2 =
            log_manager->PrepareLogEntry(NEXT_PAGE_ID_KEY, std::to_string(11))
                .value();
        log_manager->WriteLogEntry(entry2);

        auto entry3 =
            log_manager->PrepareLogEntry(NEXT_PAGE_ID_KEY, std::to_string(12))
                .value();
        log_manager->WriteLogEntry(entry3);

        return absl::OkStatus();
    }

    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
    std::unique_ptr<BufferManager> buffer_manager;
    std::unique_ptr<RecoveryManager> recovery_manager;
    std::unique_ptr<BplusTreeIndex> index;
};

TEST_F(RecoveryManagerTest, RecoverSuccess) {
    EXPECT_TRUE(Init().ok());
    EXPECT_TRUE(InsertDummyData().ok());

    auto index_root_page_id = INVALID_PAGE_ID;
    auto next_page_id_status = recovery_manager->Recover(index_root_page_id);
    EXPECT_TRUE(next_page_id_status.ok());

    EXPECT_EQ(next_page_id_status.value(), 12);

    EXPECT_EQ(index_root_page_id, INVALID_PAGE_ID);
}

}  // namespace graphchaindb
