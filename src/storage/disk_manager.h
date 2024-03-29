#ifndef STORAGE_DISK_MANAGER_H
#define STORAGE_DISK_MANAGER_H

#include <fstream>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "root_page.h"
#include "src/common/config.h"

namespace graphchaindb {

// DiskManager is responsible for reading and writing to db and log files
// todo: Thread safety?
class DiskManager {
   public:
    explicit DiskManager(absl::string_view db_path);

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    ~DiskManager();

    // Load the database from the given db path.
    //
    // MUST be called before any set/get operation on the database.
    //
    // Returns NotFoundError if the files aren't found
    absl::StatusOr<RootPage*> LoadDB();

    // Create the db and log files on disk.
    absl::StatusOr<RootPage*> CreateDBFilesAndLoadDB();

    // Write the given log entry into the log file
    absl::Status WriteLogEntry(char* log_entry, int size);

    // Read the log entry at the given offset
    // INFO: Expects ownership of the buffer to be taken by the caller
    absl::StatusOr<char*> ReadLogEntry(int offset);

    // Store the contents of the given page id into the destination buffer
    absl::Status ReadPage(page_id_t page_id, char* destination);

    // Write the contents of the given data buffer in the given page
    absl::Status WritePage(page_id_t page_id, char* data, bool flush = false);

    // Get Log file size
    int32_t GetLogFileSize();

   private:
    int32_t GetFileSize(std::string file_name);

    std::string db_path_;
    std::fstream db_file_;
    std::fstream log_file_;
};

// Exposed for testing
static constexpr absl::string_view TEST_DB_PATH = "/tmp/testdb";

}  // namespace graphchaindb

#endif  // STORAGE_DISK_MANAGER_H
