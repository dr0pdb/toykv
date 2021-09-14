#ifndef STORAGE_OPTION_H
#define STORAGE_OPTION_H

namespace graphchaindb {

// Provides options to use while loading the storage layer from disk
struct Options {
    Options();

    // creates the database if it doesn't already exists
    // defaults to false
    bool create_if_not_exists = false;

    // returns an error if the database already exists
    // defaults to true
    bool error_if_exists = true;
};

// Provides options while storing key value pairs in storage
struct WriteOptions {
    WriteOptions() = default;
};

// Provides options while reading key value pairs from storage
struct ReadOptions {
    ReadOptions() = default;
};

}  // namespace graphchaindb

#endif  // STORAGE_OPTION_H
