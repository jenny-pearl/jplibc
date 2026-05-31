#include <stdarg.h>
#include <unistd.h>

char buffer[1000000];
int64_t buffer_length = 0;

typedef enum {
	_NULL,
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

Kind parse_option(char* format, int64_t* i)
{
	switch (format[*i]) {
	case '\0':
		return _NULL;
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

void read_integer_to_buffer(int64_t number)
{
	int64_t digit_count = 1;
	int64_t temp = number / 10;

	while (temp) {
		temp /= 10;
		digit_count += 1;
	}

	int count = digit_count;

	do {
		buffer[buffer_length + digit_count - 1] = (number % 10) + '0';
		digit_count -= 1;
		number /= 10;
	} while (number);

	buffer_length += count;
}

void read_float_to_buffer(double number)
{
	int64_t whole = number;
	read_integer_to_buffer(whole);

	buffer[buffer_length] = '.';
	buffer_length += 1;

	number -= whole;

	int64_t iteration = 0;

	do {
		iteration += 1;
		number *= 10;
		whole = number;
		buffer[buffer_length] = whole + '0';
		buffer_length += 1;
		number -= whole;
	} while (number && (iteration < 15));
}

void read_hexadecimal_to_buffer(int64_t number)
{
	int64_t four_bits = 15;
	int64_t printing = 0;

	buffer[buffer_length] = '0';
	buffer_length += 1;
	buffer[buffer_length] = 'x';
	buffer_length += 1;

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
			buffer[buffer_length + counter - 1] = printing + '0';
		} else {
			buffer[buffer_length + counter - 1] = printing + 'A' - 10;
		}

		number = number >> 4;
		counter -= 1;
	} while (counter > 0);

	buffer_length += count;
}

void read_octal_to_buffer(int64_t number)
{
	int64_t three_bits = 7;
	int64_t printing = 0;

	buffer[buffer_length] = '0';
	buffer_length += 1;
	buffer[buffer_length] = 'x';
	buffer_length += 1;

	int64_t count = 0;

	int64_t temp = number;

	while (temp) {
		temp = temp >> 3;
		count += 1;
	}

	int64_t counter = count;

	do {
		printing = number & three_bits;

		buffer[buffer_length + counter - 1] = printing + '0';

		number = number >> 3;
		counter -= 1;
	} while (counter > 0);

	buffer_length += count;
}

int parse_arg_with_kind(Kind kind, va_list args)
{
	char* pointer = 0;
	int64_t sval = 0;
	double dval = 0;

	switch (kind) {
	case PERCENT:
		buffer[buffer_length] = '%';
		buffer_length += 1;
		break;
	case OCTAL:
		sval = va_arg(args, int64_t);
		read_octal_to_buffer(sval);
		break;
	case POINTER:
	case HEXADECIMAL:
		sval = va_arg(args, int64_t);
		read_hexadecimal_to_buffer(sval);
		break;
	case CHAR:
		buffer[buffer_length] = va_arg(args, char);
		buffer_length += 1;
		break;
	case CSTRING:
		pointer = va_arg(args, char *);
		while (pointer) {
			buffer[buffer_length] = *pointer;
			buffer_length += 1;
			pointer += 1;
		}
		break;
	case STRING:
		sval = va_arg(args, int);
		pointer = va_arg(args, char *);
		for (int64_t i = 0; i < sval; i += 1) {
			buffer[buffer_length] = pointer[i];
			buffer_length += 1;
		}
		break;
	case INT:
		sval = va_arg(args, int64_t);
		read_integer_to_buffer(sval);
		break;
	case FLOAT:
		dval = va_arg(args, double);
		read_float_to_buffer(dval);
		break;
	default:
		write(2, "exhaustion of type of variables to print\n", 41);
		return -1;
	}

	return -1;
}

int handle_backslash_option(char c)
{
	switch (c) {
	case '\0':
		return 1;
	case 'n':
		buffer[buffer_length] = '\n';
		buffer_length += 1;
		break;
	case 't':
		buffer[buffer_length] = '\t';
		buffer_length += 1;
		break;
	case '\\':
		buffer[buffer_length] = '\\';
		buffer_length += 1;
		break;
	default:
		return 1;
	}
}

int minprintf(char* format, ...)
{
	va_list args;
	va_start(args, format);

	for (int64_t i = 0; format[i] != '\0'; i += 1) {
		switch (format[i]) {
		case '\\': {
			i += 1;
			int error = handle_backslash_option(format[i]);
			if (error) {
				return -1;
			}
		} break;
		case '%': {
			i += 1;
			Kind kind = parse_option(format, &i);

			if (kind == __Kind_count || kind == _NULL) {
				return -1;
			}

			parse_arg_with_kind(kind, args);
		} break;
		default: {
			buffer[buffer_length] = format[i];
			buffer_length += 1;
		} break;
		}
	}

	va_end(args);

	return buffer_length;
}

void minflush(int fd)
{
	write(fd, buffer, buffer_length);
	buffer_length = 0;
}

int main(void)
{
	int64_t a = 8;

	minprintf("%o\n", a);

	minflush(1);
}
