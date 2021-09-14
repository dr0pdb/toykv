#include "src/storage/buffer_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_entry.h"
#include "src/storage/log_manager.h"

namespace graphchaindb {

class BufferManagerTest : public ::testing::Test {
   protected:
    BufferManagerTest() {
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".db");
        std::filesystem::remove(
            std::string{TEST_DB_PATH.data(), TEST_DB_PATH.size()} + ".log");
        disk_manager = std::make_unique<DiskManager>(TEST_DB_PATH);
        log_manager = std::make_unique<LogManager>(disk_manager.get());
        buffer_manager = std::make_unique<BufferManager>(disk_manager.get(),
                                                         log_manager.get());
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

        return buffer_manager->Init(STARTING_NORMAL_PAGE_ID);
    }

    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
    std::unique_ptr<BufferManager> buffer_manager;
};

TEST_F(BufferManagerTest, AllocatePageSuccess) {
    EXPECT_TRUE(Init().ok());

    for (int i = 0; i < 5; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
    }
}

}  // namespace graphchaindb
