#ifndef STORAGE_STORAGE_IMPL_H
#define STORAGE_STORAGE_IMPL_H

#include <cstdint>
#include <cstdio>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "bplus_tree_index.h"
#include "buffer_manager.h"
#include "disk_manager.h"
#include "log_manager.h"
#include "option.h"
#include "recovery_manager.h"
#include "storage.h"

namespace graphchaindb {

// An implementation of the Storage interface
class StorageImpl : public Storage {
   public:
    StorageImpl(const Options& options, absl::string_view db_path);

    StorageImpl(const StorageImpl&) = delete;
    StorageImpl& operator=(const StorageImpl&) = delete;

    ~StorageImpl() override;

    // Sets the given value corresponding to the given key.
    // overwrites the existing value if it exists.
    absl::Status Set(const WriteOptions& options, absl::string_view key,
                     absl::string_view value) override;

    // Gets the latest value corresponding to the given key.
    absl::StatusOr<std::string> Get(const ReadOptions& options,
                                    absl::string_view key) override;

    // Delete the value corresponding to the given key.
    //
    // Returns NotFoundError if the existing value doesn't exist.
    absl::Status Delete(const WriteOptions& options,
                        absl::string_view key) override;

    // Run recovery procedure.
    // Also handles create_if_not_exists and error_if_exists from options.
    absl::Status Recover(const Options& options);

   private:
    DiskManager* disk_manager_;
    LogManager* log_manager_;
    BufferManager* buffer_manager_;
    RecoveryManager* recovery_manager_;
    BplusTreeIndex* index_;
};

}  // namespace graphchaindb

#endif  // STORAGE_STORAGE_IMPL_H
