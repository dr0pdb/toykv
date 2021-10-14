#include "string_container.h"

#include <glog/logging.h>

#include "buffer_manager.h"

namespace graphchaindb {

std::string StringContainer::GetStringData(BufferManager* buffer_manager) {
    auto len = GetStringLength();
    if (len <= 60) {
        return std::string(reinterpret_cast<char*>(data_ + sizeof(int32_t)),
                           len);
    }

    absl::string_view inline_data(
        reinterpret_cast<char*>(data_ + sizeof(int32_t)), 52);

    LOG(INFO) << "StringContainer::GetStringData: inline data: " << inline_data
              << " size: " << inline_data.length();

    auto overflow_page_id =
        *reinterpret_cast<page_id_t*>(&data_[OVERFLOW_PAGE_ID_SLOT]);
    auto overflow_page_offset =
        *reinterpret_cast<int32_t*>(&data_[OVERFLOW_PAGE_OFFSET]);
    auto overflow_page_container =
        buffer_manager->GetPageWithId(overflow_page_id).value();

    overflow_page_container->AquireReadLock();

    auto overflow_page =
        reinterpret_cast<OverflowPage*>(overflow_page_container->GetData());
    auto overflown_data =
        overflow_page->GetStringAtOffset(overflow_page_offset);

    LOG(INFO) << "StringContainer::GetStringData: overflown data: "
              << overflown_data << " size: " << overflown_data.length();

    overflow_page_container->ReleaseReadLock();
    return std::string(inline_data.begin(), 52) +
           std::string(overflown_data.begin(), overflown_data.size());
}

void StringContainer::SetStringData(BufferManager* buffer_manager,
                                    absl::string_view value) {
    auto len = value.length();
    memcpy(data_, &len, sizeof(int32_t));

    if (len <= 60) {
        memcpy(data_ + sizeof(int32_t), value.begin(), value.length());
    } else {
        memcpy(data_ + sizeof(int32_t), value.begin(), 52);
        auto overflow_page_container =
            buffer_manager->GetOverflowPageWithCapacity(value.length() - 52)
                .value();
        overflow_page_container->AquireExclusiveLock();

        // set overflow page id
        auto page_id = overflow_page_container->GetPageId();
        memcpy(data_ + sizeof(int32_t) + 52, &page_id, sizeof(page_id_t));

        // set overflow page offset and write data in the page
        auto overflow_page =
            reinterpret_cast<OverflowPage*>(overflow_page_container->GetData());
        auto start_offset = overflow_page->NextSlot();
        memcpy(data_ + sizeof(int32_t) + 52 + sizeof(page_id_t), &start_offset,
               sizeof(int32_t));
        overflow_page->SetDataAtOffset(start_offset, value.begin() + 52);

        buffer_manager->UnpinPage(overflow_page_container,
                                  /* is_dirty */ true);
        overflow_page_container->ReleaseExclusiveLock();
    }
}

}  // namespace graphchaindb
