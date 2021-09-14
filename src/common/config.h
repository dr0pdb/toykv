#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <cstdint>

#include "absl/strings/string_view.h"

namespace graphchaindb {

static constexpr int PAGE_SIZE = 4096;  // size of a page in Bytes
static constexpr int STRING_CONTAINER_SIZE =
    64;  // size of the string container in Bytes
static constexpr int PAGE_BUFFER_SIZE = 50;  // size of the buffer pool cache
static constexpr int INVALID_PAGE_ID = -1;   // indicates an invalid page
static constexpr int ROOT_PAGE_ID = 0;       // id of the root database page
static constexpr int STARTING_NORMAL_PAGE_ID =
    1;  // id of the first non-special page
static constexpr int BPLUS_LEAF_KEY_VALUE_SIZE =
    30;  // maximum number of key-value pairs in a leaf node
static constexpr int BPLUS_INTERNAL_KEY_PAGE_ID_SIZE =
    49;  // maximum number of key-pageid pairs in an internal node

static constexpr int VERBOSE_CHEAP =
    1;  // verbose logging which includes should be cheap
static constexpr int VERBOSE_EXPENSIVE =
    2;  // verbose logging which might be expensive such as strings

using page_id_t = int32_t;  // page id
using ln_t = int64_t;       // log number

static constexpr ln_t INVALID_LOG_NUMBER =
    -1;  // indicates invalid log sequence number

static constexpr absl::string_view NEXT_PAGE_ID_KEY = "toykv-next-page-id";

}  // namespace graphchaindb

#endif  // COMMON_CONFIG_H
