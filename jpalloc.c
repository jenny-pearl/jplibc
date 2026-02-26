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
	bool used;  // works as a bool
	uint32_t size;
	uint32_t prev; // set to -1 to denote that there are no previous
		       // To find the start of the previous chunk
		       // pointerToThisChunk - prev
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

	if ((void*)(heapInfo.start) + heapInfo.initialSize <
		(void*)chunk - sizeof(ChunkInfo) - wantedSize) {
		return NULL;
	}

	chunk->prev = prevSize;
	chunk->used = 1;

	uint32_t nextSize = chunk->size - wantedSize - sizeof(ChunkInfo);

	chunk->size = wantedSize;

	uint32_t i = 0;

	for (char *p = (char*)chunk + sizeof(ChunkInfo); i < chunk->size; i++) {
		*(p + i) = 0;
	}

	heapInfo.available -= wantedSize + 2 * sizeof(ChunkInfo);

	ChunkInfo *next = (void*)chunk + chunk->size + sizeof(ChunkInfo);

	next->size = nextSize;
	next->used = 0;
	next->prev = wantedSize;

	return (void*)chunk + sizeof(ChunkInfo);
}

void jp_free(void *pointer)
{
	ChunkInfo *chunkToBeDeleted = (void*)pointer - sizeof(ChunkInfo);
	pointer = NULL;

	ChunkInfo *prev = (void*)chunkToBeDeleted - chunkToBeDeleted->prev - sizeof(ChunkInfo);
	ChunkInfo *next = (void*)chunkToBeDeleted + chunkToBeDeleted->size + sizeof(ChunkInfo);

	ChunkInfo *twoChunkOver = (void*)next + next->size + sizeof(ChunkInfo);

	if ((void*)twoChunkOver - (void*)heapInfo.start <= heapInfo.initialSize) {
		twoChunkOver = NULL;
	}

	if (prev->used == 0 && next->used == 0) {
		prev->size += next->size + chunkToBeDeleted->size + 2 * sizeof(ChunkInfo);

		char *p = (void*)prev + sizeof(ChunkInfo);

		for (int32_t i = 0; i < prev->size; i++) {
			*(p + i) = 0;
		}

		if (twoChunkOver != NULL) {
			twoChunkOver->prev = prev->size;
		}
	} else if (prev->used == 0 && next->used == 1) {
		prev->size += chunkToBeDeleted->size + sizeof(ChunkInfo);

		char *p = (void*)prev + sizeof(ChunkInfo);

		for (int32_t i = 0; i < prev->size; i++) {
			*(p + i) = 0;
		}

		next->prev = prev->size;
	} else if (prev->used == 1 && next->used == 0) {
		chunkToBeDeleted->size += next->size + sizeof(ChunkInfo);

		char *p = (void*)prev + sizeof(ChunkInfo);

		for (int32_t i = 0; i < chunkToBeDeleted->size; i++) {
			*(p + i) = 0;
		}

		if (twoChunkOver != NULL) {
			twoChunkOver->prev = chunkToBeDeleted->size;
		}
	} else if (prev->used == 1 && next->used == 1) {
		chunkToBeDeleted->used = 0;

		char *p = (void*)chunkToBeDeleted + sizeof(ChunkInfo);

		for (int32_t i = 0; i < chunkToBeDeleted->size; i++) {
			*(p + i) = 0;
		}
	} else {
		fprintf(stderr, "UNREACHABLE\n");
	}

	return;
}

int main(void)
{
	int *p = jp_alloc(sizeof(int));
	*p = 3;

	int *pointer = jp_alloc(sizeof(int));
	*pointer = 3;

	jp_free(pointer);
}
