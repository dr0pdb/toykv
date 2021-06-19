#include "storage.h"

namespace graphchaindb {

absl::Status Storage::Load(const Options& options, absl::string_view db_path,
                           Storage** storage) {
    return absl::OkStatus();
}

absl::Status Storage::Set(const WriteOptions& options, absl::string_view key,
                          absl::string_view value) {
    return absl::OkStatus();
}

absl::StatusOr<std::string> Storage::Get(const ReadOptions& options,
                                         absl::string_view key) {
    return absl::OkStatus();
}

Storage::~Storage() = default;

}  // namespace graphchaindb
