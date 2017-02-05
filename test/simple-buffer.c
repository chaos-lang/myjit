#include <stdarg.h>

#define BUFFER_CLEAR    0
#define BUFFER_PUT      1
#define BUFFER_GET      2

static char *simple_buffer(int op, char *format_str, ...)
{
	static char buf[4096];
	va_list ap;
	switch (op) {
		case BUFFER_CLEAR: buf[0] = '\0'; break;
		case BUFFER_PUT:
				   va_start(ap, format_str);
				   vsprintf(buf + strlen(buf), format_str, ap);
				   va_end(ap);
				   break;
	}
	return buf;
}

static char *simple_buffer_d(int op, char *format_str, double d)
{
	return simple_buffer(op, format_str, d);
}

static char *simple_buffer_dd(int op, char *format_str, double d1, double d2)
{
	return simple_buffer(op, format_str, d1, d2);
}
