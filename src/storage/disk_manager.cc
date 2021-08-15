#include "disk_manager.h"

#include <glog/logging.h>

#include <iostream>

namespace graphchaindb {

DiskManager::DiskManager(absl::string_view db_path)
    : db_path_{std::string{db_path.data(), db_path.size()}} {}

absl::Status DiskManager::LoadDB() {
    LOG(INFO) << "DiskManager::LoadDB: Start";

    db_file_.open(db_path_ + ".db",
                  std::ios::in | std::ios::binary | std::ios::out);

    log_file_.open(db_path_ + ".log", std::ios::in | std::ios::binary |
                                          std::ios::out | std::ios::app);

    if (!db_file_.is_open() || !log_file_.is_open()) {
        LOG(ERROR) << "DiskManager::LoadDB: error while "
                      "creating db and log files";

        return absl::InternalError("unable to load db and log files.");
    }

    return absl::OkStatus();
}

absl::Status DiskManager::CreateDBFilesAndLoadDB() {
    LOG(INFO) << "DiskManager::CreateDBFilesAndLoadDB: Start";

    db_file_.open(db_path_ + ".db",
                  std::ios::in | std::ios::binary | std::ios::out);

    log_file_.open(db_path_ + ".log", std::ios::in | std::ios::binary |
                                          std::ios::out | std::ios::trunc);

    if (!db_file_.is_open() || !log_file_.is_open()) {
        LOG(ERROR) << "DiskManager::CreateDBFilesAndLoadDB: error while "
                      "creating db and log files";

        return absl::InternalError(
            "unable to create and open db and log files.");
    }

    // creation done, close the fstream so that they can be loaded in a
    // different mode.
    db_file_.close();
    log_file_.close();

    return LoadDB();
}

}  // namespace graphchaindb
