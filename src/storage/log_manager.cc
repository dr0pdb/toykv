#include "log_manager.h"

namespace graphchaindb {

LogManager::LogManager(DiskManager* disk_manager)
    : disk_manager_{disk_manager} {}

}  // namespace graphchaindb
