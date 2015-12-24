#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>

void swap_init (void);
void swap_in (int idx, void *kpage);
int swap_out (void *kpage);
void swap_remove (int swap_index);

#endif /* vm/swap.h */
