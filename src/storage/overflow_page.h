#ifndef STORAGE_OVERFLOW_PAGE_H
#define STORAGE_OVERFLOW_PAGE_H

#include <glog/logging.h>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "page.h"
#include "src/common/config.h"

namespace graphchaindb {

// A simple general purpose overflow page which can contain overflown data.
//
//
// Format (size in bytes):
// --------------------------
// | Headers (12) | Content |
// --------------------------
//
// Header
// ---------------------------------------
// | PageType (4) | PageId (4) | Used(4) |
// ---------------------------------------
//
class OverflowPage {
   public:
    OverflowPage() = default;

    OverflowPage(const OverflowPage&) = delete;
    OverflowPage& operator=(const OverflowPage&) = delete;

    ~OverflowPage() = default;

    // init the page
    void InitPage(page_id_t page_id) {
        page_id_ = page_id;
        page_type_ = PageType::PAGE_TYPE_OVERFLOW;
    }

    // the next slot to insert new data in.
    // REQUIRES: exclusive lock is held on the page container.
    inline int32_t NextSlot() { return space_used; }

    // remaining capacity in the overflow page
    inline int32_t RemainingCapacity() { return PAGE_SIZE - space_used; }

    // get string at the given offset. The offset starts from the
    // data_ entry and doesn't include the header.
    absl::string_view GetStringAtOffset(int32_t offset) {
        LOG(INFO) << "OverflowPage::GetStringAtOffset: offset " << offset;
        CHECK_GE(offset, 0);

        auto size = *reinterpret_cast<int32_t*>(&data_[offset]);
        CHECK_LT(offset + size + sizeof(int32_t), DATA_SIZE);

        return absl::string_view(&data_[offset + sizeof(int32_t)], size);
    }

    // set the data at the given offset in the page. The offset starts from the
    // data_ entry and doesn't include the header.
    absl::Status SetDataAtOffset(int32_t offset, absl::string_view data) {
        LOG(INFO) << "OverflowPage::SetDataAtOffset: offset " << offset
                  << " data: " << data;
        CHECK_GE(offset, 0);

        auto size = data.length();
        CHECK_LT(offset + size + sizeof(int32_t), DATA_SIZE);

        space_used += sizeof(int32_t) + size;

        memcpy(data_ + offset, &size, sizeof(int32_t));
        memcpy(data_ + offset + sizeof(int32_t), data.begin(), size);

        return absl::OkStatus();
    }

    static constexpr int HEADER_SIZE = 12;
    static constexpr int DATA_SIZE = PAGE_SIZE - HEADER_SIZE;

   private:
    PageType page_type_;
    page_id_t page_id_;
    int32_t space_used{HEADER_SIZE};  // space including the header
    char data_[DATA_SIZE];            // the contents
};

}  // namespace graphchaindb

#endif  // STORAGE_OVERFLOW_PAGE_H