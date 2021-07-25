#include "storage_impl.h"

namespace graphchaindb {

StorageImpl::StorageImpl(const Options& options, absl::string_view db_path)
    : disk_manager_(new DiskManager(db_path)),
      log_manager_(new LogManager(disk_manager_)),
      buffer_manager_(new BufferManager(disk_manager_)),
      recovery_manager_(new RecoveryManager(log_manager_)),
      index_(new BplusTreeIndex(buffer_manager_)) {}

StorageImpl::~StorageImpl() {
    delete buffer_manager_;
    delete log_manager_;
    delete disk_manager_;
    delete recovery_manager_;
    delete index_;
}

absl::Status StorageImpl::Set(const WriteOptions& options,
                              absl::string_view key, absl::string_view value) {
    return absl::OkStatus();
}

absl::StatusOr<std::string> StorageImpl::Get(const ReadOptions& options,
                                             absl::string_view key) {
    return "hello";
}

absl::Status StorageImpl::Recover(const Options& options) {
    return absl::OkStatus();
}

}  // namespace graphchaindb
