#include "src/storage/default_key_comparator.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <memory>
#include <random>

#include "absl/strings/string_view.h"
#include "src/common/test_utils.h"

namespace graphchaindb {

TEST(DefaultComparatorTest, Success) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(10, 99);

    auto count = 1000;
    for (int i = 0; i < count; i++) {
        auto a = dist(mt);
        auto b = dist(mt);

        std::string keya = "dummy_key_" + std::to_string(a);
        std::string keyb = "dummy_key_" + std::to_string(b);

        auto comp = (new DefaultKeyComparator())->Compare(keya, keyb);
        auto expected = (a == b) ? 0 : ((a < b) ? -1 : 1);

        EXPECT_GE(comp * expected, 0);  // same sign
    }
}

}  // namespace graphchaindb
