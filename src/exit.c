#include "exit.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

__attribute__((__noreturn__)) void _exit_(int exit_code) {
	int out = fileno(stdout);
	const char *message = "Catastrophic failure exiting. Is both exit(3) and _exit(2) overridden?\nEntering an infinite loop...\n";
	size_t len = strlen(message);
	exit(exit_code);
	_exit(exit_code);
	write(out, message, len);
	while (1);
}
