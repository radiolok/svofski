#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "diags.h"

LOGLEVEL LogLevel = LOGLEVEL_INFO;

static void log(LOGLEVEL threshold, const char* fmt, va_list ap) {
	if (LogLevel >= threshold) {
		vprintf(fmt, ap);
		fflush(stdout);
	}
}

void verbose(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	log(LOGLEVEL_VERBOSE, fmt, ap);
	va_end(ap);
}

void morbose(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	log(LOGLEVEL_MORBOSE, fmt, ap);
	va_end(ap);
}

void info(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	log(LOGLEVEL_INFO, fmt, ap);
	va_end(ap);
}

void eggog(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	log(LOGLEVEL_ZERO, fmt, ap);
	va_end(ap);
	exit(1);
}
