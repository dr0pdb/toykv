#include "storage.h"

#include "storage_impl.h"

namespace graphchaindb {

absl::StatusOr<Storage*> Storage::Load(const Options& options,
                                       absl::string_view db_path) {
    StorageImpl* impl = new StorageImpl(options, db_path);

    // recover and handle create_if_not_exists and error_if_exists
    absl::Status s = impl->Recover(options);
    if (s.ok()) {
        return impl;
    }

    return s;
}

Storage::~Storage() = default;

}  // namespace graphchaindb
