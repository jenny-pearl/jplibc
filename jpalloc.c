#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>

#define PAGE_SIZE (getpagesize())
#define WORD (sizeof(void *))

typedef struct {
        uint64_t pos;
        uint64_t size;
} mem_arena;

typedef struct {
        int a;
        int b;
        float c;
} Foo;

mem_arena *jp_malloc(uint64_t size);
void jp_free(mem_arena *block);
void *jp_push_to_block(mem_arena *block, uint64_t size);

int main(int argc, char **argv)
{
        mem_arena *block = jp_malloc(PAGE_SIZE);
        Foo *foo = jp_push_to_block(block, sizeof(Foo));
        foo->a = 3;
        foo->b = 4;
        foo->c = 3.2;
        printf("%d %d %f\n", foo->a, foo->b, foo->c);
        jp_free(block);
        return 0;
}

mem_arena *jp_malloc(uint64_t len)
{
        mem_arena *block = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

        block->size = len;
        block->pos = sizeof(mem_arena);

        return block;
}

void jp_free(mem_arena *block)
{
        munmap(block, block->size);
}

uint64_t arch_align(uint64_t n, uint64_t p)
{
        return (((n) + ((p) - 1)) & (~((p) - 1)));
}

void *jp_push_to_block(mem_arena *block, uint64_t len)
{
        uint64_t aligned = arch_align(block->pos, WORD);
        uint64_t new = aligned + len;

        if (new > block->size) assert(false);

        block->pos = new;

        return ((uint8_t *)block + aligned);
}
