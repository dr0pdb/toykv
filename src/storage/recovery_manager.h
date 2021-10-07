#ifndef STORAGE_RECOVERY_MANAGER_H
#define STORAGE_RECOVERY_MANAGER_H

#include "absl/status/status.h"
#include "log_manager.h"
#include "src/common/config.h"
#include "src/storage/bplus_tree_index.h"
#include "src/storage/default_key_comparator.h"

namespace graphchaindb {

// RecoveryManager is responsible for recovery operations
//
// Not thread safe
class RecoveryManager {
   public:
    explicit RecoveryManager(LogManager* log_manager, BplusTreeIndex* index);

    RecoveryManager(const RecoveryManager&) = delete;
    RecoveryManager& operator=(const RecoveryManager&) = delete;

    ~RecoveryManager() { delete comp_; }

    // Run recovery procedure.
    absl::StatusOr<page_id_t> Recover(page_id_t& index_root_page_id);

   private:
    LogManager* log_manager_;
    KeyComparator* comp_;
    BplusTreeIndex* index_;
};

}  // namespace graphchaindb

#endif  // STORAGE_RECOVERY_MANAGER_H
