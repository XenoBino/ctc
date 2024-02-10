#ifndef XENO_CFC_EXIT_H_
#define XENO_CFC_EXIT_H_

__attribute__((__noreturn__)) void _exit_(int exit_code);
#define EXIT(code) _exit_(code);

#endif
