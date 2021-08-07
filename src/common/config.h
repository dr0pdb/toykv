#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <cstdint>

namespace graphchaindb {

static constexpr int PAGE_SIZE = 4096;  // size of a page in Bytes
static constexpr int STRING_CONTAINER_SIZE =
    64;  // size of the string container in Bytes
static constexpr int PAGE_BUFFER_SIZE = 10;  // size of the buffer pool cache
static constexpr int INVALID_PAGE_ID = -1;   // indicates an invalid page

using page_id_t = int32_t;  // page id

}  // namespace graphchaindb

#endif  // COMMON_CONFIG_H
