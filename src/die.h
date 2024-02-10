#ifndef XENO_CTC_DIE_H_
#define XENO_CTC_DIE_H_

__attribute__((__noreturn__, __format__(printf, 1, 2))) void die(const char *fmt, ...);

#endif
