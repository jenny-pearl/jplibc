#ifndef FILE_H_
#define FILE_H_

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

#include "string.h"

#define file_open(filename, ...) \
	_file_open_implementation(filename, (FileOptions) { __VA_ARGS__ })

typedef struct {
	bool write;
	bool truncate_file;
	bool create_if_doesnt_exist;
	bool log_errors;
} FileOptions;

void read_file_to_string(int fd, DynamicString* string);
int64_t file_size(int fd);
int _file_open_implementation(char* filename, FileOptions options);
void write_string_to_file(DynamicString string, int fd);
int file_close(int fd);

int file_exists(char* filename)
{
	return !(access(filename, F_OK));
}

int _file_open_implementation(char* filename, FileOptions options)
{
	int fd, oflag = 0;

	oflag |= options.write ? O_RDWR : O_RDONLY;
	oflag |= options.create_if_doesnt_exist ? O_CREAT : 0;
	oflag |= options.truncate_file ? O_TRUNC : 0;

	if (options.create_if_doesnt_exist && !file_exists(filename)) {
		fd = open(filename, oflag, 0644);
	} else {
		fd = open(filename, oflag);
	}

	if (fd == -1 && options.log_errors) {
		fprintf(stderr, "%s : ", filename);
		perror("open");
		exit(errno);
	}

	return fd;
}

int file_close(int fd)
{
	return close(fd);
}

void write_string_to_file(DynamicString string, int fd)
{
	write(fd, string.data, string.count * sizeof(char));
}

int64_t file_size(int fd)
{
	struct stat st;
	fstat(fd, &st);
	return st.st_size;
}

void read_file_to_string(int fd, DynamicString* string)
{
	int64_t size = file_size(fd);

	if (string->capacity < size) {
		string->capacity = size;
		string->data = realloc(string->data, string->capacity * sizeof(char));
	}

	int nbytes = read(fd, string->data, sizeof(char) * size);

	if (nbytes == -1) {
		perror("read");
		exit(1);
	}

	string->count += size;
}

#endif // FILE_H_
