#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#define MIN(n, p) ((n) < (p) ? (n) : (p))
#define MAX(n, p) ((n) > (p) ? (n) : (p))

#define KiB(n) ((uint64_t)n << 10)
#define MiB(n) ((uint64_t)n << 20)
#define GiB(n) ((uint64_t)n << 30)

#define WRITE_STDERR(n) write(2, n, sizeof((n)) - 1)
#define WORD (sizeof(void *))
#define ALIGN_BY_WORD(n) \
        (((uint64_t)(n) + ((uint64_t)(WORD) - 1)) & (~((uint64_t)(WORD) - 1)))

typedef struct chunk {
        uint64_t size;
        bool used;
        struct chunk *next;
        struct chunk *prev;
} ChunkInfo;

typedef struct {
        ChunkInfo *start;
        uint64_t exposed;
} HeapInfo;

void jp_free(void *start);
void *jp_alloc(uint64_t size);
int jp_heap_init(uint64_t size);
ChunkInfo *jp_set_chunk(uint64_t size);

HeapInfo *Heap = NULL;

int main(int argc, char **argv)
{
        return 0;
}

void *jp_alloc(uint64_t size)
{
        if (!Heap) {
                uint64_t aligned_size = ALIGN_BY_WORD(size);
                if (jp_heap_init(MAX(getpagesize(), aligned_size))) return NULL;
        }

        jp_set_chunk(0);
        return NULL;
}

ChunkInfo *jp_set_chunk(uint64_t size) // shift the Heap {size} further
{
        return NULL;
}

int jp_heap_init(uint64_t size)
{
        Heap = mmap(NULL,
                        size,
                        PROT_WRITE | PROT_READ,
                        MAP_ANON | MAP_PRIVATE,
                        -1, 0);
        if (!Heap) {
                WRITE_STDERR("You do not have enough memory to initialize\n");
                return -1;
        }
        Heap->start = (ChunkInfo *)Heap + sizeof(HeapInfo);
        Heap->exposed = size - sizeof(HeapInfo);
        return 0;
}

void jp_free(ChunkInfo *chunk)
{
        return;
}
