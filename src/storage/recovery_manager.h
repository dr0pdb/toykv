#ifndef STORAGE_RECOVERY_MANAGER_H
#define STORAGE_RECOVERY_MANAGER_H

#include "absl/status/status.h"
#include "log_manager.h"
#include "src/common/config.h"

namespace graphchaindb {

// RecoveryManager is responsible for recovery operations
// todo: Thread safety?
class RecoveryManager {
   public:
    explicit RecoveryManager(LogManager* log_manager);

    RecoveryManager(const RecoveryManager&) = delete;
    RecoveryManager& operator=(const RecoveryManager&) = delete;

    ~RecoveryManager() = default;

    // Run recovery procedure.
    absl::Status Recover();

   private:
    LogManager* log_manager_;
};

}  // namespace graphchaindb

#endif  // STORAGE_RECOVERY_MANAGER_H
