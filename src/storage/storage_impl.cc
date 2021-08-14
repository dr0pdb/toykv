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
    // prepare and write log

    // update in the index

    return absl::OkStatus();
}

absl::StatusOr<std::string> StorageImpl::Get(const ReadOptions& options,
                                             absl::string_view key) {
    // get from index

    return "hello";
}

absl::Status StorageImpl::Delete(const WriteOptions& options,
                                 absl::string_view key) {
    return absl::OkStatus();
}

absl::Status StorageImpl::Recover(const Options& options) {
    absl::Status s = disk_manager_->LoadDB();

    if (s.ok()) {
        if (options.error_if_exists) {
            s = absl::AlreadyExistsError("Database files already exist");
        } else {
            // do the actual recovery
        }
    } else {
        if (options.create_if_not_exists) {
            s = disk_manager_->CreateDBFilesAndLoadDB();

            // TODO: setup necessary first time info.
        } else {
            s = absl::NotFoundError("Database files not found");
        }
    }

    return s;
}

}  // namespace graphchaindb
