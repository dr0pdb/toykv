#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <cstdint>

namespace graphchaindb {

static constexpr int PAGE_SIZE = 4096;  // size of a page in Bytes

using page_id_t = int32_t;  // page id

}  // namespace graphchaindb

#endif  // COMMON_CONFIG_H
