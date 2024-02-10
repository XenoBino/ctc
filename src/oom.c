#include "oom.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

ssize_t heapless_write_uint64(int fd, uint64_t val) {
	if (!val) return write(fd, "0", 1);

	char buf[24];
	uint_least8_t index = 0;
	buf[23] = '\0';
	while (val) {
		buf[22 - index] = '0' + (val % 10);
		index++;
		val /= 10;
	}

	return write(fd, &buf[23 - index], index + 1);
}

void _oom(const char *file, const char *func, size_t line) {
	int err = fileno(stderr);
	const char *message = "Out of memory!\n";
	write(err, message, strlen(message));
	write(err, file, strlen(file));
	write(err, "::", 2);
	heapless_write_uint64(err, line);
	write(err, " at ", 4);
	write(err, func, strlen(func));
	exit(EXIT_FAILURE);
	_exit(EXIT_FAILURE);
}
