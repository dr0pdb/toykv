#ifndef STORAGE_PAGE_H
#define STORAGE_PAGE_H

#include "src/common/config.h"

namespace graphchaindb {

// Page represent a single unit of storage in the database.
//
// It is a wrapper over the actual data pages. Contains metadata fields
// which are useful for tracking the page in the cache. Note that it is not
// a base class which other pages inherit from. The actual data page is wrapped
// within this class along with metadata obtained from it.
//
// TODO: protect page with a mutex/latch and talk about thread safety.
class Page {
   public:
    Page() = default;

    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;

    virtual ~Page();

   private:
    page_id_t page_id_;
    // TODO: add more fields for cache.
    char* data[PAGE_SIZE];
};

}  // namespace graphchaindb

#endif  // STORAGE_PAGE_H