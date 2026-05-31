#include <sys/mman.h>
#include <stdint.h>

#ifndef STDLIB_H_
#define STDLIB_H_

#define WORD (sizeof(void*))

#define MiB(x) ((int64_t)x << 20)
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define DYNAMIC_SIZE(x) ((((ChunkInfo*)(((char*)(x)) - sizeof(ChunkInfo)))->size) / (sizeof((x)[0])))

typedef struct {
	int64_t size;
	int64_t remaining;
	char *map;
} HeapInfo;

typedef struct {
	int64_t size;
	int64_t prevSize;
	int64_t used; // 1 if used else 0
} ChunkInfo;

void *memory_copy(void *destination, void *source, int64_t amount)
{
	for (int64_t i = 0; i < amount; i += 1) {
		*((char*)destination + i) = *((char*)source + i);
	}

	return destination;
}

void *memory_zero(void *pointer, int64_t size)
{
	for (int64_t i = 0; i < size; i += 1) {
		*((char*)pointer + i) = 0;
	}

	return pointer;
}

int64_t align_by_machine_word(int64_t size)
{
	return (size + (WORD - 1)) & (~(WORD - 1));
}

HeapInfo heapInfo = {0};

void *alloc(int64_t size)
{
	if (heapInfo.map == 0) {
		heapInfo.map = mmap(0, MiB(16), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		heapInfo.size = MiB(16);
		heapInfo.remaining = MiB(16);

		heapInfo.remaining -= sizeof(ChunkInfo);

		ChunkInfo *preliminary = (ChunkInfo*)heapInfo.map;

		*preliminary = (ChunkInfo) {
			.size = heapInfo.remaining,
			.prevSize = -1,
			.used = 0,
		};
	}

	int64_t alignedSize = align_by_machine_word(size);

	ChunkInfo *current = (ChunkInfo*)heapInfo.map;

	while (((char*)current + sizeof(ChunkInfo) + current->size < heapInfo.map + heapInfo.size) &&
		(current->used == 1 || current->size < alignedSize)) {
		current = (ChunkInfo*)((char*)current + sizeof(ChunkInfo) + current->size);
	}

	if ((uint64_t)current->size >= alignedSize + sizeof(ChunkInfo)) {
		ChunkInfo *next = (ChunkInfo*)((char*)current + sizeof(ChunkInfo) + alignedSize);
		next->size = current->size - sizeof(ChunkInfo) - alignedSize;
		next->used = 0;
		next->prevSize = alignedSize;
		current->size = alignedSize;
	} else {
		int64_t max = MAX();
	}

	memory_zero(current + 1, current->size);

	current->used = 1;

	heapInfo.remaining -= alignedSize + sizeof(ChunkInfo);

	return current + 1;
}

void release(void *pointer)
{
	ChunkInfo *current = (ChunkInfo*)pointer - 1;
	ChunkInfo *next = 0;
	ChunkInfo *previous = 0;

	int64_t addage = current->size;

	if ((char*)pointer + current->size < heapInfo.map + heapInfo.size) {
		next = (ChunkInfo*)((char*)pointer + current->size);
	}

	if (current->prevSize != -1) {
		previous = (ChunkInfo*)((char*)current - current->prevSize - sizeof(ChunkInfo));
	}

	if (next && (next->used == 0)) {
		current->size += next->size + sizeof(ChunkInfo);
		addage += next->size + sizeof(ChunkInfo);
	}

	if (previous && (previous->used == 0)) {
		previous->size += sizeof(ChunkInfo) + current->size;
		current = previous;
		addage += previous->size + sizeof(ChunkInfo);
	}

	heapInfo.remaining += addage;

	current->used = 0;
}

void *re_alloc(void *pointer, int64_t newSize)
{
	char *temp = alloc(newSize);

	ChunkInfo *chunk = (ChunkInfo*)pointer - 1;
	int64_t oldSize = chunk->size;

	memory_copy(temp, pointer, oldSize);
	memory_zero(temp + oldSize, newSize - oldSize);

	release(pointer);

	return temp;
}

#endif // STDLIB_H_
