#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>

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

int main(int argc, char **argv)
{
        mem_arena *block = jp_malloc(PAGE_SIZE);
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
