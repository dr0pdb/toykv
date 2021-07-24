#ifndef STORAGE_STORAGE_IMPL_H
#define STORAGE_STORAGE_IMPL_H

#include <cstdint>
#include <cstdio>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "buffer_manager.h"
#include "disk_manager.h"
#include "option.h"
#include "recovery_manager.h"
#include "storage.h"

namespace graphchaindb {

// An implementation of the Storage interface
class StorageImpl : public Storage {
   public:
    StorageImpl(const Options& options, absl::string_view db_name);

    StorageImpl(const StorageImpl&) = delete;
    StorageImpl& operator=(const StorageImpl&) = delete;

    ~StorageImpl() override;

    absl::Status Set(const WriteOptions& options, absl::string_view key,
                     absl::string_view value) override;

    absl::StatusOr<std::string> Get(const ReadOptions& options,
                                    absl::string_view key) override;

   private:
    BufferManager* buffer_manager;
    DiskManager* disk_manager_;
    RecoveryManager* recovery_manager_;
    absl::Mutex mutex_;
};

}  // namespace graphchaindb

#endif  // STORAGE_STORAGE_IMPL_H
