#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define INITIAL_COUNT 64

typedef enum {
	SUCCESS,
	NULL_PTR_DEREFERENCE,
	OVERLAPPING_REGIONS,
	SAME_MEMORY_AREA,
	FAILED_STDLIB,
} ReturnValue;

typedef struct {
	uint32_t count;
	uint32_t capacity;
	char *content;
} String;

String string_from_cstr(char *format)
{
	String string = {0};

	for (uint32_t i = 0; format[i] != '\0'; i++) {
		if (string.count >= string.capacity) {
			if (string.capacity == 0) string.capacity = INITIAL_COUNT;
			else string.capacity *= 2;

			string.content = realloc(string.content, string.capacity * sizeof(char));
		}

		string.content[i] = format[i];
		string.count++;
	}

	return string;
}

char *string_copy(String *first, String *second)
{
	if (first->capacity < second->count) {
		first->capacity = second->count;
		first->content = realloc(first->content, first->capacity * sizeof(char));
	}

	first->count = second->count;

	if (first->content == NULL) {
		return first->content;
	}

	for (uint32_t i = 0; i < second->count; i++) {
		first->content[i] = second->content[i];
	}

	return first->content;
}

int8_t string_n_copy(String *first, String *second, uint64_t n)
{
	if (n > second->count) {
		n = second->count;
	}

	for (uint32_t i = 0; i < n; i++) {
		if (first->count <= first->capacity) {
			if (first->capacity == 0) first->capacity = INITIAL_COUNT;
			else first->capacity *= 2;

			first->content = realloc(first->content, first->capacity);
		}

		if (first->content == NULL) {
			return 1;
		}

		first->content[i] = second->content[i];
		first->count++;
	}

	return 0;
}

int8_t string_equal(String *first, String *second)
{
	if (first->count != second->count) {
		return 0;
	}

	for (uint64_t i = 0; i < first->count; i++) {
		if (first->content[i] != second->content[i]) {
			return 0;
		}
	}

	return 1;
}

int8_t string_concatenate(String *first, String *second)
{
	if (!first || !second) {
		return 1;
	}

	for (uint32_t i = 0; i < second->count; i++) {
		if (first->count <= first->capacity) {
			if (first->capacity == 0) first->capacity = INITIAL_COUNT;
			else first->capacity *= 2;

			first->content = realloc(first->content, first->capacity * sizeof(char));
		}

		first->content[first->count++] = second->content[i];
	}

	return 0;
}

int8_t string_n_concatenate(String *first, String *second, uint32_t n)
{
	if (!first || !second) {
		return 1;
	}

	if (n > second->count) {
		n = second->count;
	}

	for (uint32_t i = 0; i < n; i++) {
		if (first->count <= first->capacity) {
			if (first->capacity == 0) first->capacity = INITIAL_COUNT;
			else first->capacity *= 2;

			first->content = realloc(first->content, first->capacity * sizeof(char));
		}

		first->content[first->count++] = second->content[i];
	}

	return 0;
}

int8_t string_compare(String *first, String *second)
{
	for (uint64_t i = 0; i < first->count && i < second->count; i++) {
		if (first->content[i] > second->content[i]) {
			return 1;
		} else if (first->content[i] < second->content[i]) {
			return -1;
		}
	}

	if (first->count > second->count) {
		return 1;
	} else if (first->count < second->count) {
		return -1;
	}

	return 0;
}

uint64_t string_first_occurrence(String *string, char character)
{
	uint64_t i = 0;
	for (; i < string->count && string->content[i] != character; ++i);

	if (string->content[i] == character) {
		return i;
	}

	return -1;
}

uint64_t string_last_occurrence(String *string, char character)
{
	uint32_t i = 0;
	int64_t last = -1;

	for (; i < string->count; ++i) {
		if (string->content[i] == character) {
			last = i;
		}
	}

	return last;
}

uint64_t string_find_substring(String *haystack, String *needle)
{
	for (uint64_t i = 0; i < haystack->count; i++) {
		if (haystack->content[i] == needle->content[0]) {
			String temp = {
				.content = haystack->content + i,
				.count = needle->count,
				.capacity = needle->count
			};
			if (string_equal(&temp, needle)) return i;
		}
	}

	return -1;
}

String string_tokenizer(String *string, char separator)
{
	String result = {0};
	uint32_t i = 0;
	for (; i < string->count && string->content[i] != separator; i++);

	if (string->content[i] == separator) {
		result.content = string->content,
		result.count = i,
		result.capacity = i,

		string->content += i;
		string->count -= i;
		string->capacity -= i;
	} else {
		result.content = string->content;
		result.count = i - 1;
		result.capacity = i - 1;

		string->content = NULL;
		string->count = 0;
		string->capacity = 0;
	}

	return result;
}

void discard_space(String *string)
{
	while (*string->content == ' ' && string->count > 0) {
		string->content++;
		string->count--;
		string->capacity--;
	}

	return;
}

ReturnValue read_file_into_string(String *string, FILE *fp)
{
	while (!feof(fp)) {
		if (string->count >= string->capacity) {
			if (string->capacity == 0) string->capacity = INITIAL_COUNT;
			else string->capacity *= 2;

			string->content = realloc(string->content,
				string->capacity * sizeof(char));
			if (string->content == NULL) {
				return FAILED_STDLIB;
			}
		}

		string->content[string->count++] = getc(fp);
	}

	return SUCCESS;
}

int64_t string_to_signed_integer(String *string)
{
	int64_t temp = 0;
	int8_t negative = 0;
	uint32_t index = 0;

	if (*string->content == '-') {
		negative = 1;
		index++;
	}

	for (; index < string->count; index++) {
		temp *= 10;
		temp += string->content[index] - 48;
	}

	return negative == 0 ? temp : -temp;
}

uint64_t string_to_unsigned_integer(String *string)
{
	uint64_t temp = 0ULL;
	uint32_t index = 0;

	for (; index < string->count; index++) {
		temp *= 10;
		temp += string->content[index] - 48;
	}

	return temp;
}

double string_to_double(String *string)
{
	double temp = 0.0f;
	uint32_t index = 0;
	uint8_t negative = 0;

	if (*string->content == '-') {
		negative = 1;
		index++;
	}

	for (; index < string->count && string->content[index] != '.'; index++) {
		temp *= 10;
		temp += (string->content[index] - 48);
	}

	index += 1; // skipped the dot

	for (uint64_t digitValue = 10; index < string->count; index++, digitValue *= 10) {
		temp += (double)(string->content[index] - 48) / (double)digitValue;
	}

	return negative == 0 ? temp : -temp;
}

ReturnValue memory_move(void *destination, void *source, uint64_t countOfBytes)
{
	if (destination == source) {
		return SAME_MEMORY_AREA;
	}

	if (!destination || !source) {
		return NULL_PTR_DEREFERENCE;
	}

	if (destination > source && destination < source + countOfBytes) {
		return OVERLAPPING_REGIONS;
	} else if (destination < source && destination + countOfBytes > source) {
		return OVERLAPPING_REGIONS;
	}

	for (uint64_t i = 0; i < countOfBytes; i++) {
		*(destination + i) = *(source + i);
	}

	return SUCCESS;
}

ReturnValue memory_copy(void *destination, void *source, uint64_t countOfBytes)
{
	if (!destination || !source) {
		return NULL_PTR_DEREFERENCE;
	}

	for (uint64_t i = 0; i < countOfBytes; i++) {
		*(destination + i) = *(source + i);
	}

	return SUCCESS;
}

ReturnValue memory_zero(void *destination, uint64_t countOfBytes)
{
	if (destination == NULL) {
		return NULL_PTR_DEREF;
	}

	for (uint64_t i = 0; i < countOfBytes; i++) {
		*(destination + i) = 0;
	}

	return SUCCESS;
}
