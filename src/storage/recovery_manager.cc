#include "recovery_manager.h"

#include <glog/logging.h>

namespace graphchaindb {

RecoveryManager::RecoveryManager(LogManager* log_manager)
    : log_manager_{CHECK_NOTNULL(log_manager)} {}

}  // namespace graphchaindb
