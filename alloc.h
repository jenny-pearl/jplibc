#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <stdint.h>

typedef struct {
        int64_t size; // next can be reached by this->size + (char*)this + sizeof(ChunkHeader)
        int64_t prev_size; // prev can be reached (char*)this - this->prev_size - sizeof(ChunkHeader)
                            // this will be -1 in the first chunk
        bool used; // later just set up another index for the next free block maybe make these int64_t
} ChunkHeader;

typedef struct {
        int64_t reserve_size;
        int64_t commit_size;
        ChunkHeader* start;
} HeapMetadata;

HeapMetadata heap_ = {0};

int64_t align_by_machine_word(int64_t size)
{
	return (size + (sizeof(void*) - 1)) & (~(sizeof(void*) - 1));
}

void* jp_alloc(int64_t size)
{
        if (heap_.start == 0) {
                heap_.start = mmap(0, (int64_t)1 << 20,
                                        PROT_READ | PROT_WRITE,
                                        MAP_ANON | MAP_PRIVATE,
                                        -1, 0);

                if (heap_.start == MAP_FAILED) {
                        assert(false && "You are a disgrace to earth who cannot even allocate 1 MiB");
                }

                heap_.reserve_size = (int64_t)1 << 20;
                heap_.commit_size = (int64_t)1 << 20;
                *heap_.start = (ChunkHeader) {
                        .size = ((int64_t)1 << 20) - sizeof(ChunkHeader),
                        .prev_size = -1,
                        .used = 0,
                };
        }

        int64_t aligned_size = align_by_machine_word(size);

        ChunkHeader* curr = heap_.start;

        while ((char*)curr < (char*)(heap_.start) + sizeof(ChunkHeader) + heap_.commit_size &&
                        (curr->used == 1 || curr->size < aligned_size)) {
                curr = (ChunkHeader*)((char*)curr + curr->size + sizeof(ChunkHeader));
        }

        if ((char*)curr >= (char*)(heap_.start) + heap_.commit_size) {
                assert(false && "heap limit reached");
        }

        int64_t pre_allocation_size = curr->size;
        curr->used = 1;

        ChunkHeader* next = (ChunkHeader*)((char*)curr + aligned_size + sizeof(ChunkHeader));

        if (pre_allocation_size > aligned_size + (int64_t)sizeof(ChunkHeader) &&
                (char*)next + sizeof(ChunkHeader) < (char*)(heap_.start) + heap_.commit_size) {
                curr->size = aligned_size;
                next->prev_size = aligned_size;
                next->size = pre_allocation_size - aligned_size - sizeof(ChunkHeader);
                next->used = 0;
        }

        curr += 1;

        return curr;
}

void jp_free(void* chunk)
{
        ChunkHeader* delete = (ChunkHeader*)chunk - 1;
        ChunkHeader* prev = 0;
        ChunkHeader* next = 0;

        delete->used = 0;

        if (delete > heap_.start) {
                prev = (ChunkHeader*)((char*)delete - delete->prev_size - sizeof(ChunkHeader));
        }

        if ((char*)delete + delete->size + sizeof(ChunkHeader) < (char*)heap_.start + heap_.commit_size) {
                next = (ChunkHeader*)((char*)chunk + delete->size);
        }

        if (prev != 0 && prev->used == 0) {
                prev->size += delete->size + sizeof(ChunkHeader);
                delete = prev;
                delete->used = 0;
        }

        if (next != 0) {
                if (next->used == 0) {
                        delete->size += next->size + sizeof(ChunkHeader);
                        ChunkHeader* next_next = (ChunkHeader*)((char*)next + next->size + sizeof(ChunkHeader));
                        if ((char*)next_next < (char*)heap_.start + heap_.commit_size) {
                                next_next->prev_size = delete->size;
                        }
                } else {
                        next->prev_size = delete->size;
                }
        }
}
