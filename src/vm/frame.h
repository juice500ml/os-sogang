#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <list.h>
#include "threads/palloc.h"

void *get_frame (enum palloc_flags);

#endif /* vm/frame.h */
