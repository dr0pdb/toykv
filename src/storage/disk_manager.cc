#include "disk_manager.h"

#include <glog/logging.h>
#include <sys/stat.h>

#include <iostream>

#include "src/storage/log_entry.h"

namespace graphchaindb {

DiskManager::DiskManager(absl::string_view db_path)
    : db_path_{std::string{db_path.data(), db_path.size()}} {}

DiskManager::~DiskManager() {
    db_file_.close();
    log_file_.close();
}

absl::StatusOr<RootPage*> DiskManager::LoadDB() {
    LOG(INFO) << "DiskManager::LoadDB: Start at " << db_path_;

    db_file_.open(db_path_ + ".db",
                  std::ios::in | std::ios::binary | std::ios::out);

    log_file_.open(db_path_ + ".log", std::ios::in | std::ios::binary |
                                          std::ios::out | std::ios::app);

    if (!db_file_.is_open() || !log_file_.is_open()) {
        LOG(ERROR) << "DiskManager::LoadDB: error while "
                      "opening db and log files: "
                   << strerror(errno);

        // TODO: consider returning not found error
        return absl::InternalError("unable to load db and log files.");
    }

    char* root_data = new char[PAGE_SIZE];
    absl::Status s = ReadPage(ROOT_PAGE_ID, root_data);
    if (!s.ok()) {
        LOG(ERROR) << "DiskManager::LoadDB: error while "
                      "reading root page";
        return s;
    }

    RootPage* root_page = reinterpret_cast<RootPage*>(root_data);

    CHECK_EQ(root_page->GetPageId(), ROOT_PAGE_ID);
    CHECK_EQ(root_page->GetPageType(), PAGE_TYPE_ROOT);
    return root_page;
}

absl::StatusOr<RootPage*> DiskManager::CreateDBFilesAndLoadDB() {
    LOG(INFO) << "DiskManager::CreateDBFilesAndLoadDB: Start at " << db_path_;

    db_file_.open(db_path_ + ".db", std::ios::in | std::ios::binary |
                                        std::ios::out | std::ios::trunc);

    log_file_.open(db_path_ + ".log", std::ios::in | std::ios::binary |
                                          std::ios::out | std::ios::trunc);

    if (!db_file_.is_open() || !log_file_.is_open()) {
        LOG(ERROR) << "DiskManager::CreateDBFilesAndLoadDB: error while "
                      "creating db and log files: "
                   << strerror(errno);

        return absl::InternalError(
            "unable to create and open db and log files.");
    }

    std::unique_ptr<RootPage> temp_root = std::make_unique<RootPage>();
    absl::Status s =
        WritePage(ROOT_PAGE_ID, reinterpret_cast<char*>(temp_root.get()),
                  /* flush */ true);
    if (!s.ok()) {
        return s;
    }

    // creation done, close the fstream so that they can be loaded in a
    // different mode.
    db_file_.close();
    log_file_.close();

    return LoadDB();
}

absl::Status DiskManager::WriteLogEntry(char* log_entry, int size) {
    LOG(INFO) << "DiskManager::WriteLogEntry: Start";

    log_file_.write(log_entry, size);
    if (log_file_.bad()) {
        LOG(ERROR) << "DiskManager::WriteLogEntry: error while "
                      "adding log entry: "
                   << strerror(errno);
        return absl::InternalError("Unable to add log entry in the log file.");
    }
    log_file_.flush();

    return absl::OkStatus();
}

absl::StatusOr<char*> DiskManager::ReadLogEntry(int offset) {
    LOG(INFO) << "DiskManager::ReadLogEntry: Start at offset: " << offset;

    log_file_.seekp(offset, std::ios::beg);
    if (log_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadLogEntry: error while "
                      "seeking log file to the offset: "
                   << strerror(errno);
        return absl::InternalError(
            "error in seeking log file to the given offset");
    }

    char* header = new char[LogEntry::HEADER_SIZE];
    log_file_.read(header, LogEntry::HEADER_SIZE);
    if (log_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadLogEntry: error while "
                      "reading log entry header: "
                   << strerror(errno);
        return absl::InternalError(
            "error in seeking log file to the given offset");
    }

    uint32_t total_size =
        *reinterpret_cast<uint32_t*>(header + LogEntry::SIZE_OFFSET);

    log_file_.seekp(offset, std::ios::beg);
    if (log_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadLogEntry: error while "
                      "seeking log file to the offset after reading header: "
                   << strerror(errno);
        return absl::InternalError(
            "error in seeking log file to the given offset after reading "
            "header");
    }

    char* log_entry = new char[total_size];
    log_file_.read(log_entry, total_size);
    if (log_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadLogEntry: error while "
                      "reading log entry: "
                   << strerror(errno);
        return absl::InternalError("error in reading log entry");
    }

    return log_entry;
}

absl::Status DiskManager::ReadPage(page_id_t page_id, char* destination) {
    LOG(INFO) << "DiskManager::ReadPage: Start for page id: " << page_id;
    CHECK_NE(page_id, INVALID_PAGE_ID);

    int db_file_offset = page_id * PAGE_SIZE;
    db_file_.seekp(db_file_offset, std::ios::beg);
    if (db_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadPage: error while "
                      "seeking db file to a page location: "
                   << strerror(errno);
        return absl::InternalError(
            "error in seeking db file to a page location");
    }

    db_file_.read(destination, PAGE_SIZE);
    if (db_file_.bad()) {
        LOG(ERROR) << "DiskManager::ReadPage: error while "
                      "reading page from disk: "
                   << strerror(errno);
        return absl::InternalError("error in reading page from disk");
    }

    // TODO: what is read size < page size
    // TODO: what if page_id position is more than the file size?

    return absl::OkStatus();
}

absl::Status DiskManager::WritePage(page_id_t page_id, char* data, bool flush) {
    LOG(INFO) << "DiskManager::WritePage: Start for page id: " << page_id;
    CHECK_NE(page_id, INVALID_PAGE_ID);

    int64_t db_file_offset = page_id * PAGE_SIZE;
    db_file_.seekp(db_file_offset, std::ios::beg);
    if (db_file_.bad()) {
        LOG(ERROR) << "DiskManager::WritePage: error while "
                      "seeking db file to a page location: "
                   << strerror(errno);
        return absl::InternalError(
            "error in seeking db file to a page location");
    }

    db_file_.write(data, PAGE_SIZE);
    if (db_file_.bad()) {
        LOG(ERROR) << "DiskManager::WritePage: error while "
                      "writing data to a page on disk: "
                   << strerror(errno);
        return absl::InternalError("error in writing data to a page on disk");
    }

    if (flush) {
        db_file_.flush();
    }

    // TODO: what is read size < page size
    // TODO: what if page_id position is more than the file size?

    return absl::OkStatus();
}

int32_t DiskManager::GetLogFileSize() { return GetFileSize(db_path_ + ".log"); }

int32_t DiskManager::GetFileSize(std::string file_name) {
    struct stat stat_data;
    int return_code = stat(file_name.c_str(), &stat_data);
    return return_code == 0 ? static_cast<int32_t>(stat_data.st_size) : -1;
}

}  // namespace graphchaindb
