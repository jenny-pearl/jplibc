#ifndef STRING_H_
#define STRING_H_

#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "io.h"

#define SS_LIT(x) (SS){.data = (x), .count = strlen((x))}

typedef struct {
        int64_t count;
	char* data;
} SS;

typedef struct {
        int64_t count;
        int64_t capacity;
	union {
		char* data;
		char* items;
	};
} DS;

int64_t cs_length(char* format)
{
        int64_t i = 0;

        if (format == 0) {
                return 0;
        }

        for (; format[i] != '\0'; i += 1);

        return i;
}

void memory_zero(void* destination, size_t length)
{
        size_t* word = destination;
        size_t count = length / sizeof(length);

        for (size_t i = 0; i < count; i += 1) {
                *word++ = 0;
        }

        char* byte = (char*)word;
        size_t single_bytes = length % sizeof(length);

        for (size_t i = 0; i < single_bytes; i += 1) {
                *byte++ = 0;
        }
}

void memory_copy(void* destination, void* source, size_t size)
{
        size_t* dst = destination;
        size_t* src = source;
        size_t count = size / sizeof(size);

        for (size_t i = 0; i < count; i += 1) {
                *dst++ = *src++;
        }

        char* dst_byte = (char*)dst;
        char* src_byte = (char*)src;
        size_t single_bytes = size % sizeof(size_t);

        for (size_t i = 0; i < single_bytes; i += 1) {
                *dst_byte++ = *src_byte++;
        }
}

void append_cs_to_ds(DS* string, char* cs)
{
        int64_t string_length = cs_length(cs);
        int64_t available = string->capacity - string->count;

        if (available < string_length) {
                string->capacity += string_length;
                string->data = realloc(string->data, string->capacity * sizeof(char));
        }

        if (string->data == 0) return;

        memory_copy(string->data + string->count * sizeof(char), cs, string_length);
        string->count += string_length;

        return;
}

DS ds_from_cs(char* format)
{
        int64_t size = cs_length(format);

        DS string = {
                .data = malloc(sizeof(char) * size),
                .count = size,
                .capacity = size,
        };

        memory_copy(string.data, format, sizeof(char) * size);

        return string;
}

SS ss_from_cs(char* format)
{
        int64_t size = cs_length(format);

        SS string = {
                .data = malloc(sizeof(char) * size),
                .count = size,
        };

        memory_copy(string.data, format, sizeof(char) * size);

        return string;
}

bool cs_equal(char* first, char* second)
{
        int64_t i = 0;

        for (; first[i] != '\0' && second[i] != '\0'; i += 1) {
                if (first[i] != second[i]) {
                        return false;
                }
        }

        if (first[i] != '\0' || second[i] != '\0') {
                return false;
        }

        return true;
}

bool cs_n_equal(char* first, char* second, int n)
{
        int64_t i = 0;

        for (; i < n && first[i] != '\0' && second[i] != '\0'; i += 1) {
                if (first[i] != second[i]) {
                        return false;
                }
        }

        return true;
}

bool ds_equal(DS first, DS second)
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

bool ss_equal(SS first, SS second)
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

DS ds_from_ss(SS string)
{
        DS result = {
                .data = malloc(sizeof(char) * string.count),
                .count = string.count,
                .capacity = string.count,
        };

        memory_copy(result.data, string.data, string.count * sizeof(char));

        return result;
}

SS ss_from_ds(DS string)
{
        SS result = {
                .data = malloc(sizeof(char) * string.count),
                .count = string.count,
        };

        memory_copy(result.data, string.data, string.count * sizeof(char));

        return result;
}

SS tokenize_by_delimiter(SS* string, char delimiter)
{
        int64_t token_end = 0;
        int64_t remaining_start = 0;

        SS result = {
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

SS tokenize_by_function(SS* string, int(*function)(int))
{
        int64_t token_end = 0;
        int64_t remaining_start = 0;

        SS result = {
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

int append_format_string_to_ds(DS* string, char* cs, ...)
{
        va_list args;
        va_start(args, cs);

        int count = _print_implementation(NULL, cs, -1, &args);

        if (string == NULL) {
                printf("Am I a fucking dumbass\?\n");
        }

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

int ss_to_integer(SS string, int64_t* result)
{
	if (string.count > 21) {
		return 0;
	}

        int64_t index = 0;
        int64_t sign = 1;
        *result = 0;

        if (string.count == 0) {
                return 0;
        }

        if (string.data[0] == '-') {
                if (string.count == 1) {
                        return 0;
                }

                sign = -1;
                string.data += 1;
                string.count -= 1;
        }

        for (; index < string.count; index += 1) {
                *result = ((*result) * 10) + (string.data[index] - '0');
        }

        if (index != string.count) {
                return 0;
        }

        *result *= sign;

        return 1;
}

void strip_left_ss(SS* string)
{
        int i = 0;

        while (i < string->count && isspace(string->data[i])) {
                i += 1;
        }

        string->data += i;
        string->count -= i;
}

void strip_right_ss(SS* string)
{
        int i = 0;

        while (i < string->count && isspace(string->data[i])) {
                i += 1;
        }

        string->data += i;
        string->count -= i;
}

void strip_ss(SS* string)
{
        strip_left_ss(string);
        strip_right_ss(string);
}

bool ss_ends_with(SS content, SS ender)
{
        if (ender.count > content.count) return false;

        for (int64_t i = 0; i < ender.count; i += 1) {
                if (content.data[content.count - 1 - i] != ender.data[ender.count - 1 - i]) {
                        return false;
                }
        }

        return true;
}

bool ss_starts_with(SS content, SS starter)
{
        if (starter.count > content.count) return false;

        for (int64_t i = 0; i < starter.count; i += 1) {
                if (content.data[i] != starter.data[i]) {
                        return false;
                }
        }

        return true;
}

char* cs_dup(char* cs)
{
        int64_t length = cs_length(cs) + 1;
        char* new_cs = malloc(length * sizeof(char));

        memory_copy(new_cs, cs, length - 1);
        new_cs[length - 1] = '\0';

        return new_cs;
}

int ss_to_double(SS float_ss, double* result)
{
	double sign = 1;

	*result = 0;

	if (*float_ss.data == '-') {
		if (float_ss.count == 1) {
			return 0;
		}

		sign = -1;
		float_ss.data += 1;
		float_ss.count -= 1;
	}

	while (float_ss.count > 0 && *float_ss.data != '.') {
		*result += *result * 10 + *float_ss.data - '0';
		float_ss.data += 1;
		float_ss.count -= 1;
	}

	if (*float_ss.data == '.') {
		float_ss.data += 1;
		float_ss.count -= 1;

		int64_t i = 0;
		double value = 1;

		for (; i < float_ss.count; i += 1) {
			value /= 10;
			*result += value * (float_ss.data[i] - '0');
		}
	}

	*result *= sign;

	return 1;
}

int is_valid_integer(SS integer_string)
{
        if (integer_string.count <= 0) {
                return 0;
        }

	if (*integer_string.data == '-') {
		if (integer_string.count < 2) {
			return 0;
		}

		integer_string.data += 1;
		integer_string.count -= 1;
	}

	for (int64_t i = 0; i < integer_string.count; i += 1) {
		if (!isdigit(integer_string.data[i])) {
			return 0;
		}
	}

	return 1;
}

int is_valid_hex(SS hex_string)
{
	if (!cs_n_equal(hex_string.data, "0x", 2) && !cs_n_equal(hex_string.data, "0X", 2)) {
		return 0;
	}

	hex_string.data -= 2;
	hex_string.count -= 2;

	if (hex_string.count < 1) {
		return 0;
	}

	for (int64_t i = 0; i < hex_string.count; i += 1) {
		if (!isdigit(hex_string.data[i]) && hex_string.data[i] <= 'a' && hex_string.data[i] >= 'f'
			       	&& hex_string.data[i] <= 'A' && hex_string.data[i] >= 'F') {
			return 0;
		}
	}

	return 1;
}

int ss_to_hex(SS hex_string, int64_t* result)
{
	*result = 0;

	for (int64_t i = 0; i < hex_string.count; i += 1) {
		if (isdigit(hex_string.data[i])) {
			*result = *result * 16 + hex_string.data[i] - '0';
		} else if (hex_string.data[i] >= 'a' || hex_string.data[i] <= 'f') {
			*result = *result * 16 + hex_string.data[i] - 'a';
		} else if (hex_string.data[i] >= 'A' || hex_string.data[i] <= 'F') {
			*result = *result * 16 + hex_string.data[i] - 'A';
		} else {
			return 0;
		}
	}

	return 1;
}

int cs_ends_with(char* cs1, char* cs2)
{
	int64_t cs1_len = cs_length(cs1);
	int64_t cs2_len = cs_length(cs2);

	if (cs2_len > cs1_len) {
		return 0;
	}

	for (int64_t i = 0; i < cs2_len; i += 1) {
		if (cs1[(cs1_len - cs2_len) + i] != cs2[i]) {
			return 0;
		}
	}

	return 1;
}

int cs_start_with(char* cs1, char* cs2)
{
	int64_t cs1_len = cs_length(cs1);
	int64_t cs2_len = cs_length(cs2);

	if (cs2_len > cs1_len) {
		return 0;
	}

	for (int64_t i = 0; i < cs2_len; i += 1) {
		if (cs1[i] != cs2[i]) {
			return 0;
		}
	}

	return 1;
}

void append_ss_to_ds(DS* first, SS second)
{
	first->capacity += second.count;
	first->data = realloc(first->data, sizeof(char) * first->capacity);
	memory_copy(first->data + first->count, second.data, sizeof(char) * second.count);
	first->count += second.count;
}

void append_ss_to_ss(SS* first, SS second)
{
	char* temp = malloc(sizeof(char) * (first->count + second.count));
        memory_copy(temp, first->data, first->count * sizeof(char));
	memory_copy(temp + first->count, second.data, sizeof(char) * second.count);
	first->count += second.count;
        first->data = temp;
}

DS ds_copy(DS* string)
{
        DS copy = {
                .data = malloc(string->capacity),
                .count = string->count,
                .capacity = string->capacity,
        };

        memory_copy(copy.data, string->data, sizeof(char) * string->count);

        return copy;
}

char* cs_from_ss(SS string)
{
        char* cs = malloc(sizeof(char) * (string.count + 1));
        memory_copy(cs, string.data, string.count * sizeof(char));
        cs[string.count] = '\0';
        return cs;
}

#endif // STRING_H_
