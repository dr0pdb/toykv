#ifndef STORAGE_RECOVERY_MANAGER_H
#define STORAGE_RECOVERY_MANAGER_H

#include "src/common/config.h"

namespace graphchaindb {

// RecoveryManager is responsible for recovery operations
// todo: Thread safety?
class RecoveryManager {
   public:
    RecoveryManager();

    RecoveryManager(const RecoveryManager&) = delete;
    RecoveryManager& operator=(const RecoveryManager&) = delete;

    ~RecoveryManager() = default;

   private:
};

}  // namespace graphchaindb

#endif  // STORAGE_RECOVERY_MANAGER_H
