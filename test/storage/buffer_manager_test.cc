#include "src/storage/buffer_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/common/test_utils.h"
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

        log_manager->SetNextLogNumber(STARTING_LOG_NUMBER);

        return buffer_manager->Init(STARTING_NORMAL_PAGE_ID);
    }

    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
    std::unique_ptr<BufferManager> buffer_manager;
};

TEST_F(BufferManagerTest, AllocatePageWithoutEvictionSuccess) {
    EXPECT_TRUE(Init().ok());

    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
    }
}

TEST_F(BufferManagerTest, AllocatePageWithTotalEvictionSuccess) {
    EXPECT_TRUE(Init().ok());

    Page* pages[PAGE_BUFFER_SIZE];

    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
        pages[i] = page_status.value();
    }

    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        buffer_manager->UnpinPage(pages[i], false);
    }

    // we should be able to allocate more now that we've unpinned all the
    // existing ones
    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + PAGE_BUFFER_SIZE + i);
    }
}

TEST_F(BufferManagerTest, AllocatePageWithPartialEvictionSuccess) {
    EXPECT_TRUE(Init().ok());

    Page* pages[PAGE_BUFFER_SIZE];

    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
        pages[i] = page_status.value();
    }

    for (int i = 0; i < 10; i++) {
        buffer_manager->UnpinPage(pages[i], false);
    }

    // we should be able to allocate 10 more now that we've unpinned all the
    // existing ones
    for (int i = 0; i < 10; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + PAGE_BUFFER_SIZE + i);
    }

    for (int i = 20; i < 30; i++) {
        buffer_manager->UnpinPage(pages[i], false);
    }

    // we should be able to allocate 10 more now that we've unpinned all the
    // existing ones
    for (int i = 0; i < 10; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
    }
}

TEST_F(BufferManagerTest, PagePersistenceSuccess) {
    EXPECT_TRUE(Init().ok());

    Page* pages[PAGE_BUFFER_SIZE + 10];
    std::string data[PAGE_BUFFER_SIZE + 10];

    for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
        EXPECT_EQ(page_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
        pages[i] = page_status.value();

        data[i] = generate_random_string();

        memcpy(pages[i]->GetData(), data[i].c_str(), data[i].size());
    }

    for (int i = 0; i < 10; i++) {
        buffer_manager->UnpinPage(pages[i], true);
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(FLUSH_WAIT_INTERVAL_MILLISECONDS));

    // Allocate more pages to guarantee eviction of unpinned pages.
    for (int i = 0; i < 10; i++) {
        auto page_status = buffer_manager->AllocateNewPage();
        EXPECT_TRUE(page_status.ok());
    }

    // unpin these so that we can read the first 10 from disk again and verify
    // that the contents are persisted.
    for (int i = 20; i < 30; i++) {
        buffer_manager->UnpinPage(pages[i], true);
    }

    // we should be able to allocate 10 more now that we've unpinned all the
    // existing ones
    for (int i = 0; i < 10; i++) {
        auto page_container_status =
            buffer_manager->GetPageWithId(STARTING_NORMAL_PAGE_ID + i);
        EXPECT_TRUE(page_container_status.ok());

        EXPECT_EQ(page_container_status.value()->GetPageId(),
                  STARTING_NORMAL_PAGE_ID + i);
        EXPECT_EQ(memcmp(page_container_status.value()->GetData(),
                         data[i].c_str(), data[i].size()),
                  0);

        EXPECT_FALSE(page_container_status.value()->GetPageDirty());
    }
}

}  // namespace graphchaindb
