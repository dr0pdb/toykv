#include "src/storage/overflow_page.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <memory>

#include "absl/strings/string_view.h"
#include "src/common/test_utils.h"

namespace graphchaindb {

TEST(OverflowPageTest, SetGetSuccess) {
    auto overflow_page = std::make_unique<OverflowPage>();
    overflow_page->InitPage(1);

    EXPECT_TRUE(overflow_page->SetDataAtOffset(0, TEST_KEY_1).ok());

    auto second_offset = sizeof(int32_t) + TEST_KEY_1.length();
    EXPECT_TRUE(overflow_page->SetDataAtOffset(second_offset, TEST_KEY_2).ok());

    EXPECT_EQ(overflow_page->GetStringAtOffset(0), TEST_KEY_1);
    EXPECT_EQ(overflow_page->GetStringAtOffset(second_offset), TEST_KEY_2);

    EXPECT_EQ(overflow_page->RemainingCapacity(),
              OverflowPage::DATA_SIZE - TEST_KEY_1.length() -
                  TEST_KEY_2.length() - 2 * sizeof(int32_t));
}

}  // namespace graphchaindb
