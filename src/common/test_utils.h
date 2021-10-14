#ifndef COMMON_TEST_UTILS_H
#define COMMON_TEST_UTILS_H

#include <random>
#include <string>

#include "absl/strings/string_view.h"

namespace graphchaindb {

// Exposed for testing
static constexpr absl::string_view TEST_KEY_1 = "test_key_1";
static constexpr absl::string_view TEST_KEY_2 = "test_key_2";
static constexpr absl::string_view TEST_VALUE_1 = "test_value_1";
static constexpr absl::string_view TEST_VALUE_2 = "test_value_2";
static constexpr absl::string_view TEST_VALUE_LONG =
    "test_value_long_"
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static std::string generate_random_string_size(int size) {
    const std::string VALID_CHARS =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, VALID_CHARS.size() - 1);

    std::string random_string;
    std::generate_n(std::back_inserter(random_string), size,
                    [&]() { return VALID_CHARS[distribution(generator)]; });

    return random_string;
}

static std::string generate_random_string() {
    return generate_random_string_size(50);
}

}  // namespace graphchaindb

#endif  // COMMON_TEST_UTILS_H
