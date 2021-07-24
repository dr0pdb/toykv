#ifndef STORAGE_PAGE_H
#define STORAGE_PAGE_H

namespace graphchaindb {

class Page {
   public:
    Page() = default;

    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;

    virtual ~Page();
};

}  // namespace graphchaindb

#endif  // STORAGE_PAGE_H