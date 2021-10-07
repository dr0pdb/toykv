#include "recovery_manager.h"

#include <glog/logging.h>

namespace graphchaindb {

RecoveryManager::RecoveryManager(LogManager* log_manager, BplusTreeIndex* index)
    : log_manager_{CHECK_NOTNULL(log_manager)},
      comp_{new DefaultKeyComparator()},
      index_{CHECK_NOTNULL(index)} {}

absl::StatusOr<page_id_t> RecoveryManager::Recover(
    page_id_t& index_root_page_id) {
    LOG(INFO) << "RecoveryManager::Recover: Init with index_root_page_id: "
              << index_root_page_id;

    auto next_log_number = STARTING_LOG_NUMBER;
    auto next_page_id = STARTING_NORMAL_PAGE_ID;
    index_root_page_id = INVALID_PAGE_ID;
    WriteOptions recovery_write_options;
    absl::Status s;

    auto log_entry_iterator = log_manager_->GetLogEntryIterator();
    while (log_entry_iterator->IsValid()) {
        auto current_entry_or_status = log_entry_iterator->GetCurrent();
        if (!current_entry_or_status.ok()) {
            return current_entry_or_status.status();
        }
        auto current_entry = std::move(current_entry_or_status.value());

        next_log_number = current_entry->GetLogNumber() + 1;
        switch (current_entry->GetType()) {
            case LOG_ENTRY_SET:
                if (comp_->Compare(current_entry->GetKey(), NEXT_PAGE_ID_KEY) ==
                    0) {
                    auto value_str = current_entry->GetValue().value();
                    std::string value(value_str);
                    next_page_id = std::stoull(value);
                    break;
                }

                if (comp_->Compare(current_entry->GetKey(),
                                   INDEX_ROOT_PAGE_ID_KEY) == 0) {
                    auto value_str = current_entry->GetValue().value();
                    std::string value(value_str);
                    index_root_page_id = std::stoull(value);
                    break;
                }

                s = index_->Set(recovery_write_options, current_entry->GetKey(),
                                current_entry->GetValue().value());
                if (!s.ok()) {
                    LOG(ERROR)
                        << "RecoveryManager::Recover: error in set operation";
                    return s;
                }
                break;

            case LOG_ENTRY_DELETE:
                s = index_->Delete(recovery_write_options,
                                   current_entry->GetKey());
                if (!s.ok()) {
                    LOG(ERROR) << "RecoveryManager::Recover: error in delete "
                                  "operation";
                    return s;
                }
                break;

            default:
                LOG(ERROR)
                    << "RecoveryManager::Recover: invalid log record type "
                    << current_entry->GetType();
                return absl::InternalError(
                    "RecoveryManager::Recover: invalid log record type");
        }

        s = log_entry_iterator->Next();
        if (!s.ok()) {
            LOG(ERROR) << "RecoveryManager::Recover: error while calling Next "
                          "on log iterator";
            return s;
        }
    }

    log_manager_->SetNextLogNumber(next_log_number);
    return next_page_id;
}

}  // namespace graphchaindb
