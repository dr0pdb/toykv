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
    static absl::StatusOr<Storage*> Load(const Options& options,
                                         absl::string_view db_path);

    // Sets the given value corresponding to the given key.
    // overwrites the existing value if it exists.
    virtual absl::Status Set(const WriteOptions& options, absl::string_view key,
                             absl::string_view value) = 0;

    // Delete the value corresponding to the given key.
    //
    // Returns NotFoundError if the existing value doesn't exist.
    virtual absl::Status Delete(const WriteOptions& options,
                                absl::string_view key) = 0;

    // Gets the latest value corresponding to the given key.
    virtual absl::StatusOr<std::string> Get(const ReadOptions& options,
                                            absl::string_view key) = 0;
};

}  // namespace graphchaindb

#endif  // STORAGE_STORAGE_H
