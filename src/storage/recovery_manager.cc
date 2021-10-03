#include "recovery_manager.h"

#include <glog/logging.h>

namespace graphchaindb {

RecoveryManager::RecoveryManager(LogManager* log_manager)
    : log_manager_{CHECK_NOTNULL(log_manager)},
      comp_{new DefaultKeyComparator()} {}

absl::StatusOr<page_id_t> RecoveryManager::Recover() {
    auto next_log_number = STARTING_LOG_NUMBER;
    auto next_page_id = STARTING_NORMAL_PAGE_ID;

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
                }
                break;

            case LOG_ENTRY_DELETE:
                break;

            default:
                // Log and return error. crash the program. The logs are corrupt
                break;
        }

        log_entry_iterator->Next();
    }

    log_manager_->SetNextLogNumber(next_log_number);
    return next_page_id;
}

}  // namespace graphchaindb
