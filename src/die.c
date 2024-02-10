#include "die.h"
#include "exit.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

__attribute__((__noreturn__, __format__(printf, 1, 2))) void die(const char *fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);
	_exit_(EXIT_FAILURE);
}
