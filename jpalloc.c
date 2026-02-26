#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>

#define PAGE_SIZE getpagesize()
#define SIZE 50
#define WORD sizeof(void*)

#define MiB(x) ((uint32_t)(x) << 20)

typedef struct {
	bool used;
	uint32_t prev; // set to -1 to denote that there are no previous
		       // To find the start of the previous chunk
		       // pointerToThisChunk - prev
	uint32_t size;
} ChunkInfo;

typedef struct {
	ChunkInfo *start;
	uint32_t available;
	uint32_t initialSize;
} HeapInfo;

HeapInfo heapInfo = {0};

void *jp_alloc(uint32_t wantedSize)
{
	if (heapInfo.start == NULL) {
		heapInfo.start = mmap(NULL, MiB(1), PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0);
		heapInfo.initialSize = MiB(1);
		heapInfo.available = heapInfo.initialSize - sizeof(ChunkInfo);
		heapInfo.start->used = 0;
		heapInfo.start->prev = -1;
		heapInfo.start->size = heapInfo.available;
	}

	if (heapInfo.available < wantedSize) {
		fprintf(stderr, "Not enough memory\n");
		return NULL;
	}

	ChunkInfo *chunk = heapInfo.start;

	uint32_t prevSize = -1;

	while ((chunk->used == 1 || chunk->size < wantedSize) &&
		(void*)chunk - (void*)(heapInfo.start) <= heapInfo.initialSize) {
		prevSize = chunk->size;
		chunk = (void*)chunk + sizeof(ChunkInfo) + chunk->size;
	}

	if (chunk - heapInfo.start == heapInfo.initialSize) {
		return NULL;
	}

	chunk->prev = prevSize;
	chunk->used = 1;

	uint32_t preAllocationSize = chunk->size;

	chunk->size = wantedSize;

	for (char *p = (char*)chunk, int32_t i = 0; i < chunk->size; i++) {
		*p = 0;
	}

	heapInfo.available -= wantedSize + sizeof(ChunkInfo);

	ChunkInfo *next = (void*)chunk + chunk->size + sizeof(ChunkInfo);

	next->size = preAllocationSize;
	next->used = 0;
	next->prev = sizeof(ChunkInfo) + wantedSize;

	return chunk + 1;
}

void jp_free(void *pointer)
{
	// Check for chunks before and after the freed chunk to avoid fragmentation
	return;
}

int main(void)
{
	int *p = jp_alloc(sizeof(int));
	printf("%p\n", p);
	p = (void *)p - sizeof(ChunkInfo);
	printf("%p\n", p);
	printf("%u\n", ((ChunkInfo *)p)->size);
	printf("%d\n", ((ChunkInfo *)p)->used);
	printf("%d\n", ((ChunkInfo *)p)->prev);

	int *pointer = jp_alloc(sizeof(int));
	printf("%p\n", pointer);
	pointer = (void *)pointer - sizeof(ChunkInfo);
	printf("%p\n", pointer);
	printf("%u\n", ((ChunkInfo *)pointer)->size);
	printf("%d\n", ((ChunkInfo *)pointer)->used);
	printf("%d\n", ((ChunkInfo *)pointer)->prev);
}
