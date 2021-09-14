#ifndef COMMON_TEST_VALUES_H
#define COMMON_TEST_VALUES_H

#include "absl/strings/string_view.h"

namespace graphchaindb {

// Exposed for testing
static constexpr absl::string_view TEST_KEY_1 = "test_key_1";
static constexpr absl::string_view TEST_KEY_2 = "test_key_2";
static constexpr absl::string_view TEST_VALUE_1 = "test_value_1";
static constexpr absl::string_view TEST_VALUE_2 = "test_value_2";

}  // namespace graphchaindb

#endif  // COMMON_TEST_VALUES_H
