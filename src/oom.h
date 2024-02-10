#ifndef XENO_CTC_OOM_H_
#define XENO_CTC_OOM_H_

#include <stddef.h>

void _oom(const char *file, const char *func, size_t line);
#define oom() _oom(__FILE__, __func__, __LINE__)

#endif
