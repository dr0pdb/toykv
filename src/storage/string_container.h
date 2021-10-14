#ifndef STORAGE_STRING_KEY_H
#define STORAGE_STRING_KEY_H

#include "absl/strings/string_view.h"
#include "overflow_page.h"
#include "src/common/config.h"

namespace graphchaindb {

class BufferManager;

// A fixed length string container that handles extra length using overflow
// pages.
//
// Total size: 64 bytes
//
// Format (size in bytes):
//
// If length of string <= 60 ASCII characters
// --------------------------
// | Length (4) | data (60) |
// --------------------------
//
// If length of string > 60 ASCII characters
// ----------------------------------------------------------------------------
// | Length (4) | data (52) | Overflow page id (4) | Offset overflow page (4) |
// ----------------------------------------------------------------------------
//
class StringContainer {
   public:
    StringContainer() = default;

    // TODO: come back to this.
    StringContainer(const StringContainer&) = delete;
    StringContainer& operator=(const StringContainer&) = default;

    // Get the string length
    inline int32_t GetStringLength() {
        return *reinterpret_cast<int32_t*>(data_);
    }

    // Get the string data stored in the container
    std::string GetStringData(BufferManager* buffer_manager);

    // Set the string data stored in the container
    void SetStringData(BufferManager* buffer_manager, absl::string_view value);

    inline void EraseStringData() { memset(data_, 0, sizeof(int32_t)); }

   private:
    static constexpr int OVERFLOW_PAGE_ID_SLOT = 56;
    static constexpr int OVERFLOW_PAGE_OFFSET = 60;

    char data_[STRING_CONTAINER_SIZE];
};

}  // namespace graphchaindb

#endif  // STORAGE_STRING_KEY_H