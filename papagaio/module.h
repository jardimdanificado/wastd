/*
 * papagaio.h — Struct global "papagaio" com métodos agrupados por módulo.
 *
 * Uso:
 *   #include "papagaio.h"
 *
 *   papagaio.math.sin(1.57);
 *   papagaio.str.strcpy(dst, src);
 *   papagaio.mem.malloc(64);
 *   papagaio.io.printf("hello %d\n", 42);
 *   papagaio.ctype.isdigit('9');
 *
 * Todas as chamadas são ponteiros de função que apontam diretamente para
 * as implementações reais — debug completo, stack trace intacto, sem macros
 * com lógica embutida.
 */

#ifndef PAPAGAIO_H
#define PAPAGAIO_H

#include "papagaio.c"   /* inclui headers + implementações do SDK */

/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: math
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* classificação */
    int    (*isinf)    (double x);
    int    (*isnan)    (double x);
    int    (*isfinite) (double x);

    /* trigonometria */
    double (*sin)  (double x);
    double (*cos)  (double x);
    double (*tan)  (double x);
    double (*asin) (double x);
    double (*acos) (double x);
    double (*atan) (double x);
    double (*atan2)(double y, double x);

    /* hiperbólicas */
    double (*sinh) (double x);
    double (*cosh) (double x);
    double (*tanh) (double x);

    /* exponencial e logaritmo */
    double (*exp)  (double x);
    double (*exp2) (double x);
    double (*log)  (double x);
    double (*log2) (double x);
    double (*log10)(double x);
    double (*pow)  (double base, double exponent);

    /* raiz e potência */
    double (*sqrt) (double x);
    double (*cbrt) (double x);
    double (*hypot)(double x, double y);

    /* arredondamento */
    double (*floor)(double x);
    double (*ceil) (double x);
    double (*round)(double x);
    double (*trunc)(double x);
    double (*fmod) (double x, double y);

    /* valor absoluto e sinal */
    double (*fabs)    (double x);
    double (*copysign)(double x, double y);

    /* versões float */
    float  (*sinf)  (float x);
    float  (*cosf)  (float x);
    float  (*tanf)  (float x);
    float  (*sqrtf) (float x);
    float  (*fabsf) (float x);
    float  (*floorf)(float x);
    float  (*ceilf) (float x);
    float  (*roundf)(float x);
    float  (*powf)  (float base, float exp);
    float  (*logf)  (float x);
    float  (*expf)  (float x);
} Papagaio_Math;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: str  (string.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* comprimento */
    size_t (*strlen) (const char *s);
    size_t (*strnlen)(const char *s, size_t maxlen);
    char  *(*strdup) (const char *s);

    /* cópia */
    char  *(*strcpy) (char *dst, const char *src);
    char  *(*strncpy)(char *dst, const char *src, size_t n);

    /* concatenação */
    char  *(*strcat) (char *dst, const char *src);
    char  *(*strncat)(char *dst, const char *src, size_t n);

    /* comparação */
    int    (*strcmp) (const char *s1, const char *s2);
    int    (*strncmp)(const char *s1, const char *s2, size_t n);

    /* busca */
    char  *(*strchr) (const char *s, int c);
    char  *(*strrchr)(const char *s, int c);
    char  *(*strstr) (const char *haystack, const char *needle);
    char  *(*strpbrk)(const char *s, const char *accept);

    /* span */
    size_t (*strspn) (const char *s, const char *accept);
    size_t (*strcspn)(const char *s, const char *reject);

    /* tokenização */
    char  *(*strtok)  (char *str, const char *delim);
    char  *(*strtok_r)(char *str, const char *delim, char **saveptr);

    /* extras */
    char  *(*strrev)  (char *s);
    const char *(*strerror)(int errnum);
} Papagaio_Str;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: mem  (memória — parte de string.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    void  *(*memcpy) (void *dst, const void *src, size_t n);
    void  *(*memmove)(void *dst, const void *src, size_t n);
    void  *(*memset) (void *s, int c, size_t n);
    int    (*memcmp) (const void *s1, const void *s2, size_t n);
    void  *(*memchr) (const void *s, int c, size_t n);

    /* alocação (stdlib) */
    void  *(*malloc) (size_t size);
    void  *(*calloc) (size_t nmemb, size_t size);
    void  *(*realloc)(void *ptr, size_t size);
    void   (*free)   (void *ptr);
} Papagaio_Mem;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: io  (stdio.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int (*printf)        (const char *fmt, ...);
    int (*sprintf)       (char *buf, const char *fmt, ...);
    int (*snprintf)      (char *buf, size_t size, const char *fmt, ...);
    int (*vprintf)       (const char *fmt, va_list ap);
    int (*vsprintf)      (char *buf, const char *fmt, va_list ap);
    int (*vsnprintf)     (char *buf, size_t size, const char *fmt, va_list ap);
    int (*putchar)       (int c);
    int (*puts)          (const char *s);
    int (*fprintf_stdout)(const char *fmt, ...);
    int (*fprintf_stderr)(const char *fmt, ...);
} Papagaio_IO;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: conv  (conversões — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* string → número */
    int               (*atoi)  (const char *s);
    long              (*atol)  (const char *s);
    long long         (*atoll) (const char *s);
    long              (*strtol)  (const char *s, char **endptr, int base);
    long long         (*strtoll) (const char *s, char **endptr, int base);
    unsigned long     (*strtoul) (const char *s, char **endptr, int base);
    unsigned long long(*strtoull)(const char *s, char **endptr, int base);

    /* número → string */
    char *(*itoa)(int value, char *str, int base);
} Papagaio_Conv;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: algo  (algoritmos — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    void  (*qsort)  (void *base, size_t nmemb, size_t size,
                     int (*compar)(const void *, const void *));
    void *(*bsearch)(const void *key, const void *base, size_t nmemb, size_t size,
                     int (*compar)(const void *, const void *));

    /* aritmética inteira */
    int       (*abs)  (int x);
    long      (*labs) (long x);
    long long (*llabs)(long long x);

    div_t   (*div)  (int numer, int denom);
    ldiv_t  (*ldiv) (long numer, long denom);
    lldiv_t (*lldiv)(long long numer, long long denom);
} Papagaio_Algo;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: rng  (pseudo-aleatório — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int  (*rand) (void);
    void (*srand)(unsigned int seed);
} Papagaio_RNG;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: ctype  (ctype.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int (*isalpha) (int c);
    int (*isupper) (int c);
    int (*islower) (int c);
    int (*isdigit) (int c);
    int (*isalnum) (int c);
    int (*isxdigit)(int c);
    int (*isspace) (int c);
    int (*isblank) (int c);
    int (*ispunct) (int c);
    int (*iscntrl) (int c);
    int (*isprint) (int c);
    int (*isgraph) (int c);
    int (*toupper) (int c);
    int (*tolower) (int c);
} Papagaio_Ctype;


/* ═══════════════════════════════════════════════════════════════════════════
 * Struct raiz: Papagaio
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    Papagaio_Math  math;
    Papagaio_Str   str;
    Papagaio_Mem   mem;
    Papagaio_IO    io;
    Papagaio_Conv  conv;
    Papagaio_Algo  algo;
    Papagaio_RNG   rng;
    Papagaio_Ctype ctype;
} Papagaio;


/* ═══════════════════════════════════════════════════════════════════════════
 * Instância global (const — todos os ponteiros são fixos em compile-time)
 * ═════════════════════════════════════════════════════════════════════════ */

static const Papagaio papagaio = {

    /* ── math ────────────────────────────────────────────────────────────── */
    .math = {
        .isinf     = isinf,
        .isnan     = isnan,
        .isfinite  = isfinite,

        .sin   = sin,
        .cos   = cos,
        .tan   = tan,
        .asin  = asin,
        .acos  = acos,
        .atan  = atan,
        .atan2 = atan2,

        .sinh  = sinh,
        .cosh  = cosh,
        .tanh  = tanh,

        .exp   = exp,
        .exp2  = exp2,
        .log   = log,
        .log2  = log2,
        .log10 = log10,
        .pow   = pow,

        .sqrt  = sqrt,
        .cbrt  = cbrt,
        .hypot = hypot,

        .floor = floor,
        .ceil  = ceil,
        .round = round,
        .trunc = trunc,
        .fmod  = fmod,

        .fabs     = fabs,
        .copysign = copysign,

        .sinf   = sinf,
        .cosf   = cosf,
        .tanf   = tanf,
        .sqrtf  = sqrtf,
        .fabsf  = fabsf,
        .floorf = floorf,
        .ceilf  = ceilf,
        .roundf = roundf,
        .powf   = powf,
        .logf   = logf,
        .expf   = expf,
    },

    /* ── str ─────────────────────────────────────────────────────────────── */
    .str = {
        .strlen  = strlen,
        .strnlen = strnlen,
        .strdup  = strdup,

        .strcpy  = strcpy,
        .strncpy = strncpy,

        .strcat  = strcat,
        .strncat = strncat,

        .strcmp  = strcmp,
        .strncmp = strncmp,

        .strchr  = strchr,
        .strrchr = strrchr,
        .strstr  = strstr,
        .strpbrk = strpbrk,

        .strspn  = strspn,
        .strcspn = strcspn,

        .strtok   = strtok,
        .strtok_r = strtok_r,

        .strrev   = strrev,
        .strerror = strerror,
    },

    /* ── mem ─────────────────────────────────────────────────────────────── */
    .mem = {
        .memcpy  = memcpy,
        .memmove = memmove,
        .memset  = memset,
        .memcmp  = memcmp,
        .memchr  = memchr,

        .malloc  = malloc,
        .calloc  = calloc,
        .realloc = realloc,
        .free    = free,
    },

    /* ── io ──────────────────────────────────────────────────────────────── */
    .io = {
        .printf         = printf,
        .sprintf        = sprintf,
        .snprintf       = snprintf,
        .vprintf        = vprintf,
        .vsprintf       = vsprintf,
        .vsnprintf      = vsnprintf,
        .putchar        = putchar,
        .puts           = puts,
        .fprintf_stdout = fprintf_stdout,
        .fprintf_stderr = fprintf_stderr,
    },

    /* ── conv ────────────────────────────────────────────────────────────── */
    .conv = {
        .atoi    = atoi,
        .atol    = atol,
        .atoll   = atoll,
        .strtol  = strtol,
        .strtoll = strtoll,
        .strtoul = strtoul,
        .strtoull= strtoull,
        .itoa    = itoa,
    },

    /* ── algo ────────────────────────────────────────────────────────────── */
    .algo = {
        .qsort   = qsort,
        .bsearch = bsearch,
        .abs     = abs,
        .labs    = labs,
        .llabs   = llabs,
        .div     = div,
        .ldiv    = ldiv,
        .lldiv   = lldiv,
    },

    /* ── rng ─────────────────────────────────────────────────────────────── */
    .rng = {
        .rand  = rand,
        .srand = srand,
    },

    /* ── ctype ───────────────────────────────────────────────────────────── */
    .ctype = {
        .isalpha  = isalpha,
        .isupper  = isupper,
        .islower  = islower,
        .isdigit  = isdigit,
        .isalnum  = isalnum,
        .isxdigit = isxdigit,
        .isspace  = isspace,
        .isblank  = isblank,
        .ispunct  = ispunct,
        .iscntrl  = iscntrl,
        .isprint  = isprint,
        .isgraph  = isgraph,
        .toupper  = toupper,
        .tolower  = tolower,
    },
};

#endif /* PAPAGAIO_H */