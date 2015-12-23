#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <list.h>

struct page
  {
    void *upage;
    void *kpage;
    struct thread *th;
    struct list_elem elem;
    bool writable;
  };

bool install_page (void *upage, void *kpage, bool writable);
void add_page (void *kpage);
void destroy_page_by_thread (struct thread *t);

#endif /* vm/page.h */
