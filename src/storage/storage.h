#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <cstdint>
#include <cstdio>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "option.h"

namespace graphchaindb {

// A persistent key value storage layer
// It is thread safe
class Storage {
   public:
    Storage() = default;

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    virtual ~Storage();

    // Loads the storage with the given name.
    // Stores the pointer to the created storage in the "storage" variable
    static absl::Status Load(const Options& options, absl::string_view db_path,
                             Storage** storage);

    virtual absl::Status Set(const WriteOptions& options, absl::string_view key,
                             absl::string_view value);

    virtual absl::StatusOr<std::string> Get(const ReadOptions& options,
                                            absl::string_view key);
};

}  // namespace graphchaindb

#endif  // STORAGE_STORAGE_H
