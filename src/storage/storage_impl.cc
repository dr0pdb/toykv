#include "storage_impl.h"

#include <glog/logging.h>

#include "src/storage/log_entry.h"

namespace graphchaindb {

StorageImpl::StorageImpl(const Options& options, absl::string_view db_path)
    : disk_manager_(new DiskManager(db_path)),
      log_manager_(new LogManager(disk_manager_)),
      buffer_manager_(new BufferManager(disk_manager_, log_manager_)),
      index_(new BplusTreeIndex(buffer_manager_, disk_manager_, log_manager_)),
      recovery_manager_(new RecoveryManager(log_manager_, index_)) {}

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

    s = index_->Set(options, key, value);
    if (!s.ok()) {
        LOG(ERROR) << "StorageImpl::Set: error in set operation";
        return s;
    }
    return s;
}

absl::Status StorageImpl::Delete(const WriteOptions& options,
                                 absl::string_view key) {
    LOG(INFO) << "StorageImpl::Delete: Start with key: " << key;

    absl::StatusOr<std::unique_ptr<LogEntry>> sOrLogEntry =
        log_manager_->PrepareLogEntry(key, absl::nullopt);
    if (!sOrLogEntry.ok() || *sOrLogEntry == nullptr) {
        LOG(ERROR)
            << "StorageImpl::Delete: unable to prepare log entry for delete "
               "operation";
        return sOrLogEntry.status();
    }

    absl::Status s = log_manager_->WriteLogEntry(*sOrLogEntry);
    if (!s.ok()) {
        LOG(ERROR)
            << "StorageImpl::Delete: unable to write log entry for delete "
               "operation";
        return s;
    }

    s = index_->Delete(options, key);
    if (!s.ok()) {
        LOG(ERROR) << "StorageImpl::Delete: error in delete operation";
        return s;
    }

    return s;
}

absl::StatusOr<std::string> StorageImpl::Get(const ReadOptions& options,
                                             absl::string_view key) {
    return index_->Get(options, key);
}

absl::Status StorageImpl::Recover(const Options& options) {
    LOG(INFO) << "StorageImpl::Recover: Start";

    absl::StatusOr<RootPage*> s = disk_manager_->LoadDB();
    RootPage* root_page = nullptr;
    auto next_page_id = INVALID_PAGE_ID;
    auto bplus_tree_index_root_page_id = INVALID_PAGE_ID;

    if (s.ok()) {
        root_page = *s;

        if (options.error_if_exists) {
            LOG(ERROR) << "StorageImpl::Recover: database files already exist";
            return absl::AlreadyExistsError("Database files already exist");
        }
    } else {
        // TODO: consider checking if the error is not found error
        if (options.create_if_not_exists) {
            s = disk_manager_->CreateDBFilesAndLoadDB();
            if (!s.ok()) {
                return s.status();
            }

            root_page = *s;
        } else {
            LOG(ERROR) << "StorageImpl::Recover: database files not found";
            return s.status();
        }
    }

    LOG(INFO) << "StorageImpl::Recover: Done creating/loading DB files. ";

    auto recovery_status_or_next_page_id =
        recovery_manager_->Recover(bplus_tree_index_root_page_id);
    if (!recovery_status_or_next_page_id.ok()) {
        LOG(ERROR) << "StorageImpl::Recover: error during recovery operation.";
        return recovery_status_or_next_page_id.status();
    }

    next_page_id = recovery_status_or_next_page_id.value();
    CHECK_NE(next_page_id, INVALID_PAGE_ID);

    auto buffer_init_status = buffer_manager_->Init(next_page_id);
    if (!buffer_init_status.ok()) {
        LOG(ERROR)
            << "StorageImpl::Recover: error while initing the buffer manager";
        return buffer_init_status;
    }

    index_->Init(bplus_tree_index_root_page_id);

    return s.status();
}

}  // namespace graphchaindb
