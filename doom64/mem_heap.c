
#include <ultra64.h>
#include "i_main.h"

#define MEM_HEAP_SIZE (0x27B510) // [Immorpher] was 2.41 MB (26B510) but slightly increased for new code
u64 mem_heap[MEM_HEAP_SIZE / sizeof(u64)]; // 800BA2F0
