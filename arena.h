#ifndef ARENA_H_
#define ARENA_H_

#include <sys/mman.h>
#include <stdint.h>

#define ALIGN_TO_WORD(n) (((n) + ((WORD) - 1)) & (~((WORD) - 1)))

#define KIB(x) ((int64_t)(x) << 10)
#define MIB(x) ((int64_t)(x) << 20)
#define GIB(x) ((int64_t)(x) << 30)

#define WORD sizeof(void*)

typedef struct {
	int64_t reserve_size;
	int64_t standard_chunk_step;

	int64_t commit_size;
	int64_t pointer;
} arena;

void memory_zero(void* pointer, size_t length);
void* arena_push(arena* chunk, int64_t size);
arena* arena_create(int64_t reserve_size, int64_t commit_size);
void arena_free(arena* chunk);

void memory_zero(void* destination, size_t length)
{
	size_t* word = destination;
	size_t count = length / sizeof(length);

	for (int64_t i = 0; i < count; i += 1) {
		*word = 0;
		word += 1;
	}

	char* byte = word;
	size_t single_bytes = length % sizeof(length);

	for (int64_t i = 0; i < single_bytes; i += 1) {
		*byte = 0;
		byte += 1;
	}
}

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

void* arena_push(arena* chunk, int64_t size)
{
	int64_t aligned = ALIGN_TO_WORD(size);
	int64_t available = chunk->commit_size - chunk->pointer;

	while (available < aligned) {
		assert(commit_size + standard_chunk_step < reserve_size);
		mprotect((char*)chunk + commit_size, standard_chunk_step, PROT_WRITE | PROT_READ);
		commit_size += standard_chunk_step;
		available += standard_chunk_step;
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
