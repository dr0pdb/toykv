#include "storage_impl.h"

namespace graphchaindb {

StorageImpl::StorageImpl(const Options& options, absl::string_view db_name) {}

StorageImpl::~StorageImpl() {
    delete disk_manager_;
    delete recovery_manager_;
}

absl::Status StorageImpl::Set(const WriteOptions& options,
                              absl::string_view key, absl::string_view value) {
    return absl::OkStatus();
}

absl::StatusOr<std::string> StorageImpl::Get(const ReadOptions& options,
                                             absl::string_view key) {
    return "hello";
}

}  // namespace graphchaindb
