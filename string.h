#include <stdint.h>
#include <stdlib.h>

#ifndef STRING_H_
#define STRING_H_

typedef struct {
	int64_t count;
	int8_t* items;
} sv;

typedef struct {
	int64_t count;
	int64_t capacity;
	int8_t* items;
} string;

void memory_zero(void* destination, size_t length)
{
	size_t* word = destination;
	size_t count = length / sizeof(length);

	for (int64_t i = 0; i < count; i += 1) {
		*word++ = 0;
	}

	char* byte = word;
	size_t single_bytes = length % sizeof(length);

	for (int64_t i = 0; i < single_bytes; i += 1) {
		*byte++ = 0;
	}
}

void *memory_copy(void *destination, void *source, int64_t size)
{
	if (!destination || !source) {
		return NULL;
	}

	for (int64_t i = 0; i < size; i += 1) {
		*(char *)(destination + i) = *(char *)(source + i);
	}

	return destination;
}

int64_t cstr_length(char *format)
{
	int64_t i = 0;

	for (; format[i] != '\0'; i += 1)
		;

	return i;
}

string string_from_cstr(char *format)
{
	int64_t size = cstr_length(format);

	string str = {
		.capacity = size,
		.count = size,
		.content = malloc(sizeof(char) * size),
	};

	memory_copy(str.content, format, sizeof(char) * size);

	return str;
}

sv sv_from_cstr(char *format)
{
	int64_t size = cstr_length(format);

	sv view = {
		.count = size,
		.content = malloc(sizeof(char) * size),
	};

	memory_copy(view.content, format, sizeof(char) * size);

	return view;
}

void string_concat_string(string *first, string second)
{
	if (second.count == 0) {
		return;
	}

	int64_t prevSize = first->count;

	first->count += second.count;

	if (first->count > first->capacity) {
		first->capacity = first->count;
		first->content = realloc(first->content, first->capacity * sizeof(char));
	}

	memory_copy(first->content + prevSize, second.content, second.count);
}

int64_t cstr_compare(char *first, char *second)
{
	int64_t i = 0;

	for (; first[i] != '\0' && second[i] != '\0'; i += 1) {
		if (first[i] > second[i]) {
			return 1;
		} else if (first[i] < second[i]) {
			return -1;
		}
	}

	if (first[i] != '\0') {
		return 1;
	} else if (second[i] != '\0') {
		return -1;
	}

	return 0;
}

int64_t string_compare(string first, string second)
{
	int64_t i = 0;

	for (; i < first.count && i < second.count; i += 1) {
		if (first.content[i] > second.content[i]) {
			return 1;
		} else if (first.content[i] < second.content[i]) {
			return -1;
		}
	}

	if (i < first.count) {
		return 1;
	} else if (i < second.count) {
		return -1;
	}

	return 0;
}

int64_t sv_compare(sv first, sv second)
{
	int64_t i = 0;

	for (; i < first.count && i < second.count; i += 1) {
		if (first.content[i] > second.content[i]) {
			return 1;
		} else if (first.content[i] < second.content[i]) {
			return -1;
		}
	}

	if (i < first.count) {
		return 1;
	} else if (i < second.count) {
		return -1;
	}

	return 0;
}

int64_t string_symbol_occur(string whole, char c)
{
	int64_t index = 0;

	while (whole.content[index] != c) {
		index += 1;
	}

	return index;
}

#endif // STRING_H_
