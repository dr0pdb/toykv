#include "storage_impl.h"

#include <glog/logging.h>

#include "src/storage/log_entry.h"

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
    LOG(INFO) << "StorageImpl::Set: Start with key: " << key
              << " value: " << value;

    absl::StatusOr<std::unique_ptr<LogEntry>> sOrLogEntry =
        log_manager_->PrepareLogEntry(key, value);
    if (!sOrLogEntry.ok() || *sOrLogEntry == nullptr) {
        LOG(ERROR) << "StorageImpl::Set: unable to prepare log entry for set "
                      "operation";
        return sOrLogEntry.status();
    }

    absl::Status s = log_manager_->WriteLogEntry(*sOrLogEntry);
    if (!s.ok()) {
        LOG(ERROR)
            << "StorageImpl::Set: unable to write log entry for set operation";
        return s;
    }

    // update in the index

    return s;
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
    LOG(INFO) << "StorageImpl::Recover: Start";

    absl::Status s = disk_manager_->LoadDB();
    if (s.ok()) {
        if (options.error_if_exists) {
            LOG(ERROR) << "StorageImpl::Recover: database files already exist";
            s = absl::AlreadyExistsError("Database files already exist");
        } else {
            // do the actual recovery
        }
    } else {
        if (options.create_if_not_exists) {
            s = disk_manager_->CreateDBFilesAndLoadDB();

            // TODO: setup necessary first time info.
        } else {
            LOG(ERROR) << "StorageImpl::Recover: database files not found";
            s = absl::NotFoundError("Database files not found");
        }
    }

    LOG(INFO) << "StorageImpl::Recover: Done creating/loading DB files";

    s = log_manager_->Init();
    return s;
}

}  // namespace graphchaindb
