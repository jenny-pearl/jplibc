#ifndef IO_H_
#define IO_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define bool _Bool

char standard_io_buffer[1 << 20];
int64_t standard_io_buffer_length = 0;

typedef enum {
	_null,
	PERCENT,
	OCTAL,
	HEXADECIMAL,
	CHAR,
	CSTRING,
	STRING,
	FLOAT,
	INT,
	POINTER,
	__Kind_count,
} Kind;

Kind parse_option(char* format, int64_t* i);
int flush(int fd);
int read_integer_to_buffer(char* buffer, int64_t* buffer_length, int64_t number);
int read_float_to_buffer(char* buffer, int64_t* buffer_length, double number);
int read_hexadecimal_to_buffer(char* buffer, int64_t* buffer_length, int64_t number);
int read_octal_to_buffer(char* buffer, int64_t* buffer_length, int64_t number);
int parse_arg_with_kind(char* buffer, int64_t* buffer_length, Kind kind, va_list args);
int _print_implementation(char* string, char* format, va_list args);
int print(char* format, ...);
int fprint(int fd, char* format, ...);
int sprint(char* buffer, char* format, ...);

Kind parse_option(char* format, int64_t* i)
{
	switch (format[*i]) {
	case '\0':
		return _null;
	case 'p':
		return POINTER;
	case 'o':
		return OCTAL;
	case 'x':
		return HEXADECIMAL;
	case '%':
		return PERCENT;
	case 'c':
		return CHAR;
	case 's':
		return CSTRING;
	case 'l':
		return STRING;
	case 'd':
		return INT;
	case 'f':
		return FLOAT;
	default:
		return __Kind_count;
	}
}

int flush(int fd)
{
	write(fd, standard_io_buffer, standard_io_buffer_length * sizeof(char));
	int result = standard_io_buffer_length;
	standard_io_buffer_length = 0;
	return result;
}

int read_integer_to_buffer(char* buffer, int64_t* buffer_length, int64_t number)
{
	int64_t digit_count = 1;
	int64_t temp = number / 10;

	while (temp) {
		temp /= 10;
		digit_count += 1;
	}

	int count = digit_count;

	do {
		buffer[*buffer_length + digit_count - 1] = (number % 10) + '0';
		digit_count -= 1;
		number /= 10;
	} while (number);

	*buffer_length += count;

	return count;
}

int read_float_to_buffer(char* buffer, int64_t* buffer_length, double number)
{
	int64_t whole = number;
	int64_t whole_count = read_integer_to_buffer(buffer, buffer_length, whole);

	buffer[*buffer_length] = '.';
	*buffer_length += 1;

	number -= whole;

	int64_t iteration = 0;

	do {
		iteration += 1;
		number *= 10;
		whole = number;
		buffer[*buffer_length] = whole + '0';
		*buffer_length += 1;
		number -= whole;
	} while (number && (iteration < 15));

	return whole_count + 15;
}

int read_hexadecimal_to_buffer(char* buffer, int64_t* buffer_length, int64_t number)
{
	int64_t four_bits = 15;
	int64_t printing = 0;

	buffer[*buffer_length] = '0';
	*buffer_length += 1;
	buffer[*buffer_length] = 'x';
	*buffer_length += 1;

	int64_t count = 0;

	int64_t temp = number;

	while (temp) {
		temp = temp >> 4;
		count += 1;
	}

	int64_t counter = count;

	do {
		printing = number & four_bits;

		if (printing < 10) {
			buffer[*buffer_length + counter - 1] = printing + '0';
		} else {
			buffer[*buffer_length + counter - 1] = printing + 'A' - 10;
		}

		number = number >> 4;
		counter -= 1;
	} while (counter > 0);

	*buffer_length += count;

	return count;
}

int read_octal_to_buffer(char* buffer, int64_t* buffer_length, int64_t number)
{
	int64_t three_bits = 7;
	int64_t printing = 0;

	buffer[*buffer_length] = '0';
	*buffer_length += 1;
	buffer[*buffer_length] = 'x';
	*buffer_length += 1;

	int64_t count = 0;

	int64_t temp = number;

	while (temp) {
		temp = temp >> 3;
		count += 1;
	}

	int64_t counter = count;

	do {
		printing = number & three_bits;

		buffer[*buffer_length + counter - 1] = printing + '0';

		number = number >> 3;
		counter -= 1;
	} while (counter > 0);

	*buffer_length += count;

	return count;
}

int parse_arg_with_kind(char* buffer, int64_t* buffer_length, Kind kind, va_list args)
{
	char* pointer = 0;
	int64_t sval = 0;
	double dval = 0;

	switch (kind) {
	case PERCENT:
		buffer[*buffer_length] = '%';
		*buffer_length += 1;
		break;
	case OCTAL:
		sval = va_arg(args, int64_t);
		read_octal_to_buffer(buffer, buffer_length, sval);
		break;
	case POINTER:
	case HEXADECIMAL:
		sval = va_arg(args, int64_t);
		read_hexadecimal_to_buffer(buffer, buffer_length, sval);
		break;
	case CHAR:
		buffer[*buffer_length] = va_arg(args, char);
		*buffer_length += 1;
		break;
	case CSTRING:
		pointer = va_arg(args, char*);
		while (pointer) {
			buffer[*buffer_length] = *pointer;
			*buffer_length += 1;
			pointer += 1;
		}
		break;
	case STRING:
		sval = va_arg(args, int64_t);
		pointer = va_arg(args, char*);
		for (int64_t i = 0; i < sval; i += 1) {
			buffer[*buffer_length] = pointer[i];
			*buffer_length += 1;
		}
		break;
	case INT:
		sval = va_arg(args, int64_t);
		read_integer_to_buffer(buffer, buffer_length, sval);
		break;
	case FLOAT:
		dval = va_arg(args, double);
		read_float_to_buffer(buffer, buffer_length, dval);
		break;
	default:
		write(1, "exhaustion of type of variables to print\n", 41 * sizeof(char));
		return -1;
	}

	return -1;
}

int print(char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = _print_implementation(NULL, format, args);
	flush(1);
	return count;
}

int fprint(int fd, char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = _print_implementation(NULL, format, args);\
	flush(fd);
	return count;
}

int sprint(char* buffer, char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = _print_implementation(buffer, format, args);
	return count;
}

int _print_implementation(char* string, char* format, va_list args)
{
	if (string == NULL) {
		string = standard_io_buffer;
	}

	int64_t string_length = 0;

	for (int64_t i = 0; format[i] != '\0'; i += 1) {
		switch (format[i]) {
		case 37: {
			i += 1;
			Kind kind = parse_option(format, &i);

			if (kind == __Kind_count || kind == _null) {
				return -1;
			}

			parse_arg_with_kind(string, &string_length, kind, args);
		} break;
		default: {
			string[string_length] = format[i];
			string_length += 1;
		} break;
		}
	}

	if (string == standard_io_buffer) {
		standard_io_buffer_length = string_length;
	}

	va_end(args);

	return string_length;
}

#endif // IO_H_
