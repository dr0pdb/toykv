#ifndef STORAGE_PAGE_H
#define STORAGE_PAGE_H

#include <glog/logging.h>

#include <algorithm>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "src/common/config.h"

namespace graphchaindb {

// Indicates the type of page.
// This is stored in the header of each page.
enum PageType {
    PAGE_TYPE_INVALID,
    PAGE_TYPE_ROOT,
    PAGE_TYPE_BPLUS_INTERNAL,
    PAGE_TYPE_BPLUS_LEAF,
    PAGE_TYPE_BPLUS_OVERFLOW,
    PAGE_TYPE_STRING_OVERFLOW
};

// Page represent a single unit of storage in the database.
//
// It is a wrapper over the actual data pages. Contains metadata fields
// which are useful for tracking the page in the cache. Note that it is not
// a base class which other pages inherit from. The actual data page is wrapped
// within this class along with metadata obtained from it.
//
// This class is thread safe.
class Page {
    friend class BufferManager;

   public:
    Page() { ZeroOut(); };

    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;

    ~Page() = default;

    // Get the page id
    inline page_id_t GetPageId() { return page_id_; }

    // Get the actual page data
    inline char* GetData() { return data_; }

    // Get the dirty page flag value
    inline bool GetPageDirty() { return is_page_dirty_; }

    // Set the value of dirty flag
    // TODO: Do we need this?
    inline void SetPageDirty(bool new_value) { is_page_dirty_ = new_value; }

    // Aquire a read lock on the page
    inline void AquireReadLock() {
        LOG(INFO) << "Page::AquireReadLock: page_id " << page_id_;
        mu_.ReaderLock();
    }

    // Release the read lock on the page
    inline void ReleaseReadLock() {
        LOG(INFO) << "Page::ReleaseReadLock: page_id " << page_id_;
        mu_.ReaderUnlock();
    }

    // Aquire an exclusive lock on the page
    inline void AquireExclusiveLock() {
        LOG(INFO) << "Page::AquireExclusiveLock: page_id " << page_id_;
        mu_.Lock();
    }

    // Release the exclusive lock on the page
    inline void ReleaseExclusiveLock() {
        LOG(INFO) << "Page::ReleaseExclusiveLock: page_id " << page_id_;
        mu_.Unlock();
    }

   private:
    inline void ZeroOut() { memset(data_, 0, PAGE_SIZE); }

    page_id_t page_id_;
    absl::Mutex mu_;
    int pin_count_ = 0;
    bool is_page_dirty_ = false;
    char data_[PAGE_SIZE] GUARDED_BY(mu_);
};

}  // namespace graphchaindb

#endif  // STORAGE_PAGE_H