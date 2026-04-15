#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>


/* ── printf família ──────────────────────────────────────────────────────── */
int printf (const char *fmt, ...);
int fprintf_stdout(const char *fmt, ...);   /* stdout explícito */
int fprintf_stderr(const char *fmt, ...);   /* stderr           */

int sprintf (char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t size, const char *fmt, ...);

int vprintf (const char *fmt, va_list ap);
int vsprintf (char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

/* ── putchar / puts ──────────────────────────────────────────────────────── */
int putchar(int c);
int puts   (const char *s);


#endif /* _STDIO_H */
