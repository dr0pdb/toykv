#include "src/storage/string_container.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/config.h"
#include "src/common/test_utils.h"
#include "src/storage/buffer_manager.h"
#include "src/storage/disk_manager.h"
#include "src/storage/log_entry.h"
#include "src/storage/log_manager.h"
#include "src/storage/option.h"

namespace graphchaindb {

class StringContainerTest : public ::testing::Test {
   protected:
    StringContainerTest() {
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

        auto s2 = buffer_manager->Init(STARTING_NORMAL_PAGE_ID);
        if (!s2.ok()) {
            return s2;
        }

        log_manager->SetNextLogNumber(1);
    }

    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<LogManager> log_manager;
    std::unique_ptr<BufferManager> buffer_manager;
};

TEST_F(StringContainerTest, LessThan60StoreAndGetSuccess) {
    auto string_container = std::make_unique<StringContainer>();
    Init();

    string_container->SetStringData(buffer_manager.get(), TEST_VALUE_1);

    CHECK_EQ(string_container->GetStringLength(), TEST_VALUE_1.length());
    CHECK_EQ(string_container->GetStringData(buffer_manager.get()),
             TEST_VALUE_1);
}

TEST_F(StringContainerTest, GreaterThan60StoreAndGetSuccess) {
    auto string_container = std::make_unique<StringContainer>();
    Init();

    string_container->SetStringData(buffer_manager.get(), TEST_VALUE_LONG);

    CHECK_EQ(string_container->GetStringLength(), TEST_VALUE_LONG.length());
    auto data = string_container->GetStringData(buffer_manager.get());
    CHECK_EQ(data, TEST_VALUE_LONG);
}

TEST_F(StringContainerTest, LessThan60ClearAndSetAgainSuccess) {
    auto string_container = std::make_unique<StringContainer>();
    Init();

    string_container->SetStringData(buffer_manager.get(), TEST_VALUE_1);

    CHECK_EQ(string_container->GetStringLength(), TEST_VALUE_1.length());
    CHECK_EQ(string_container->GetStringData(buffer_manager.get()),
             TEST_VALUE_1);

    string_container->EraseStringData();
    CHECK_EQ(string_container->GetStringLength(), 0);

    string_container->SetStringData(buffer_manager.get(), TEST_VALUE_2);

    CHECK_EQ(string_container->GetStringLength(), TEST_VALUE_2.length());
    CHECK_EQ(string_container->GetStringData(buffer_manager.get()),
             TEST_VALUE_2);
}

}  // namespace graphchaindb
