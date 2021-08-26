#include "src/storage/disk_manager.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <memory>

#include "absl/strings/string_view.h"

namespace graphchaindb {

class DiskManagerTest : public ::testing::Test {
   protected:
    DiskManagerTest() {
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

}  // namespace graphchaindb
