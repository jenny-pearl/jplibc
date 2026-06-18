#ifndef STRING_H_
#define STRING_H_

#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"

#define bool _Bool
#define true 1
#define false 0

typedef struct {
	int64_t count;
	char* data;
} StaticString;

typedef struct {
	int64_t count;
	int64_t capacity;
	char* data;
} DynamicString;

char scratch_buffer[8 * 1024];

int64_t cstring_length(const char* format)
{
	int64_t i = 0;

	for (; format[i] != '\0'; i += 1);

	return i;
}

void memory_zero(void* destination, size_t length)
{
	size_t* word = destination;
	size_t count = length / sizeof(length);

	for (int64_t i = 0; i < count; i += 1) {
		*word++ = 0;
	}

	char* byte = (char*)word;
	size_t single_bytes = length % sizeof(length);

	for (int64_t i = 0; i < single_bytes; i += 1) {
		*byte++ = 0;
	}
}

void memory_copy(void *destination, void *source, size_t size)
{
	size_t* dst = destination;
	size_t* src = source;
	size_t count = size / sizeof(size);

	for (int64_t i = 0; i < count; i += 1) {
		*dst++ = *src++;
	}

	char* dst_byte = (char*)dst;
	char* src_byte = (char*)src;
	size_t single_bytes = size % sizeof(size);

	for (int64_t i = 0; i < single_bytes; i += 1) {
		*dst_byte++ = *src_byte++;
	}
}

void append_cstring_to_dynamic_string(char* cstring, DynamicString* string)
{
	int64_t string_length = cstring_length(cstring);
	int64_t available = string->capacity - string->count;

	if (available < string_length) {
		string->capacity += string_length;
		string->data = realloc(string->data, string->capacity * sizeof(char));
	}

	if (string->data == 0) return;

	memory_copy(string->data + string->count * sizeof(char), cstring, string_length);

	return;
}

DynamicString dynamic_string_from_cstring(char* format)
{
	int64_t size = cstring_length(format);

	DynamicString string = {
		.data = malloc(sizeof(char) * size),
		.count = size,
		.capacity = size,
	};

	memory_copy(string.data, format, sizeof(char) * size);

	return string;
}

StaticString static_string_from_cstring(const char* format)
{
	int64_t size = cstring_length(format);

	StaticString string = {
		.data = malloc(sizeof(char) * size),
		.count = size,
	};

	memory_copy(string.data, format, sizeof(char) * size);

	return string;
}

bool dynamic_string_equal(DynamicString first, DynamicString second)
{
	if (first.count != second.count) {
		return false;
	}

	for (int64_t i = 0; i < first.count; i += 1) {
		if (first.data[i] != second.data[i]) {
			return false;
		}
	}

	return true;
}

bool static_string_equal(StaticString first, StaticString second)
{
	if (first.count != second.count) {
		return false;
	}

	for (int64_t i = 0; i < first.count; i += 1) {
		if (first.data[i] != second.data[i]) {
			return false;
		}
	}

	return true;
}

DynamicString dynamic_string_from_static_string(StaticString string)
{
	DynamicString result = {
		.data = malloc(sizeof(char) * string.count),
		.count = string.count,
		.capacity = string.count,
	};

	memory_copy(string.data, result.data, string.count * sizeof(char));

	return result;
}

StaticString static_string_from_dynamic_string(DynamicString string)
{
	StaticString result = {
		.data = malloc(sizeof(char) * string.count),
		.count = string.count,
	};

	memory_copy(result.data, string.data, string.count * sizeof(char));

	return result;
}

StaticString tokenize_by_delimiter(StaticString* string, char delimiter)
{
	int64_t token_end = 0;
	int64_t remaining_start = 0;

	StaticString result = {
		.data = string->data,
		.count = 0,
	};

	for (; token_end < string->count && string->data[token_end] != delimiter; token_end += 1);

	for (remaining_start = token_end;
		remaining_start < string->count && string->data[remaining_start] == delimiter;
		remaining_start += 1);

	string->data += remaining_start;
	string->count -= remaining_start;

	result.count += token_end;

	return result;
}

StaticString tokenize_by_function(StaticString* string, int(*function)(int))
{
	int64_t token_end = 0;
	int64_t remaining_start = 0;

	StaticString result = {
		.data = string->data,
		.count = 0,
	};

	for (; token_end < string->count && !function(string->data[token_end]); token_end += 1);

	for (remaining_start = token_end;
		remaining_start < string->count && function(string->data[remaining_start]);
		remaining_start += 1);

	string->data += remaining_start;
	string->count -= remaining_start;

	result.count += token_end;

	return result;
}

int append_format_string_to_dynamic_string(DynamicString* string, char* cstring, ...)
{
	va_list args;
	va_start(args, cstring);

	int count = _print_implementation(NULL, cstring, args);

	int available = string->capacity - string->count;

	if (available < count) {
		string->capacity += count;
		string->data = realloc(string->data, string->capacity);
	}

	standard_io_buffer_length = 0;

	memory_copy(string->data + string->count, standard_io_buffer, count);

	string->count += count;

	return count;
}

#endif // STRING_H_
