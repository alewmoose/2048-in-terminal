#include <stdio.h>
#include <stdarg.h>
#include "logit.h"

void logit(char *fmt, ...)
{
	static FILE *logf = NULL;
	if (!logf) {
		logf = fopen("log", "w");
		if (!logf)
			return;
	}
	va_list p;
	va_start(p, fmt);
	vfprintf(logf, fmt, p);
	fflush(logf);
	va_end(p);
}
