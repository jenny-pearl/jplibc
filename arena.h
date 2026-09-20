#ifndef ARENA_H_
#define ARENA_H_

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>

#include "string.h"

#define ALIGN_TO_WORD(n) (((n) + ((sizeof(void*)) - 1)) & (~((sizeof(void*)) - 1)))

#define KIB(x) ((int64_t)(x) << 10)
#define MIB(x) ((int64_t)(x) << 20)
#define GIB(x) ((int64_t)(x) << 30)

typedef struct {
	int64_t reserve_size;
	int64_t standard_chunk_step;

	int64_t commit_size;
	int64_t pointer;
} arena;

void* arena_push(arena* chunk, int64_t size);
arena* arena_create(int64_t reserve_size, int64_t commit_size, int64_t standard_chunk_step);
void arena_free(arena* chunk);

arena* arena_create(int64_t reserve_size, int64_t commit_size, int64_t standard_chunk_step)
{
	arena* result = mmap(0, reserve_size, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);

	mprotect(result, commit_size, PROT_READ | PROT_WRITE);

	*result = (arena) {
		.reserve_size = reserve_size,
		.standard_chunk_step = standard_chunk_step,
		.commit_size = commit_size,
		.pointer = sizeof(arena),
	};

	return result;
}

void arena_pop(arena* chunk)
{
	chunk->pointer = 0;
}

void* arena_push_not_aligned(arena* chunk, int64_t size)
{
	int64_t available = chunk->commit_size - chunk->pointer;

	if (available < size) {
		assert(chunk->commit_size + chunk->standard_chunk_step < chunk->reserve_size);
		mprotect((char*)chunk + chunk->commit_size, chunk->standard_chunk_step, PROT_WRITE | PROT_READ);
		chunk->commit_size += chunk->standard_chunk_step;
		available += chunk->standard_chunk_step;
	}

	arena* place = (arena*)((char*)chunk + chunk->pointer);
	chunk->pointer += size;

	return place;
}

void* arena_push(arena* chunk, int64_t size)
{
	int64_t aligned = ALIGN_TO_WORD(size);
	int64_t available = chunk->commit_size - chunk->pointer;

	if (available < aligned) {
		assert(chunk->commit_size + chunk->standard_chunk_step < chunk->reserve_size);
		mprotect((char*)chunk + chunk->commit_size, chunk->standard_chunk_step, PROT_WRITE | PROT_READ);
		chunk->commit_size += chunk->standard_chunk_step;
		available += chunk->standard_chunk_step;
	}

	arena* place = (arena*)((char*)chunk + chunk->pointer);
	chunk->pointer += aligned;

	return place;
}

void arena_free(arena* chunk)
{
	if (chunk) { munmap(chunk, chunk->reserve_size); }
}

#endif // ARENA_H_
