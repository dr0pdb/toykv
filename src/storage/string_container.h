#ifndef STORAGE_STRING_KEY_H
#define STORAGE_STRING_KEY_H

#include "absl/strings/string_view.h"
#include "src/common/config.h"

namespace graphchaindb {

// TODO: Implement overflow page structure. Also think about who'll keep track
// of overflow pages.

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
    inline absl::string_view GetStringData() {
        // TODO: handle overflow page structure
        return reinterpret_cast<char*>(data_ + sizeof(int32_t));
    }

    // Set the string data stored in the container
    inline void SetStringData(absl::string_view value) {
        auto len = value.length();
        memcpy(data_, &len, sizeof(int32_t));

        if (len <= 60) {
            memcpy(data_ + sizeof(int32_t), value.begin(), value.length());
        } else {
            // TODO: implement overflow handling
        }
    }

    inline void EraseStringData() { memset(data_, 0, sizeof(int32_t)); }

   private:
    char data_[STRING_CONTAINER_SIZE];
};

}  // namespace graphchaindb

#endif  // STORAGE_STRING_KEY_H