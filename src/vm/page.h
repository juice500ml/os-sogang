#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>

bool install_page (void *upage, void *kpage, bool writable);

#endif /* vm/page.h */
