/*
 * test_main.c — full test suite for wasm-libc v3
 *
 * Covers: string, ctype, malloc, stdlib, printf, sscanf, math, locale, time, errno
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <locale.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

/* ── framework ───────────────────────────────────────────────────────────── */

static int _total = 0, _pass = 0, _fail = 0;

static void _check(int ok, const char *expr, const char *file, int line) {
    _total++;
    if (ok) { printf("  OK   %s\n", expr); _pass++; }
    else     { printf("  FAIL %s  (%s:%d)\n", expr, file, line); _fail++; }
}

#define CHECK(expr)       _check(!!(expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(a,b)     _check((a)==(b),  #a " == " #b, __FILE__, __LINE__)
#define CHECK_STR(a,b)    _check(strcmp((a),(b))==0, #a " == \"" #b "\"", __FILE__, __LINE__)
#define CHECK_NEAR(a,b,e) _check(fabs((double)(a)-(double)(b))<=(e), \
                              #a " ~= " #b, __FILE__, __LINE__)
#define SECTION(s)        printf("\n[%s]\n", s)

/* ── string ──────────────────────────────────────────────────────────────── */

static void test_string(void) {
    SECTION("string");
    char buf[64], s[64];

    CHECK_EQ(strlen("hello"), 5u);
    CHECK_EQ(strlen(""), 0u);
    CHECK_EQ(strnlen("hello", 3), 3u);

    memset(buf, 0xAA, 16);
    CHECK(buf[0] == (char)0xAA && buf[15] == (char)0xAA);
    memcpy(buf, "Hello, WASM!", 13);
    CHECK_EQ(memcmp(buf, "Hello, WASM!", 12), 0);

    char ov[] = "ABCDEFGH";
    memmove(ov + 2, ov, 6);
    CHECK_EQ(memcmp(ov + 2, "ABCDEF", 6), 0);
    char ov2[] = "ABCDEFGH";
    memmove(ov2, ov2 + 2, 6);
    CHECK_EQ(memcmp(ov2, "CDEFGH", 6), 0);

    CHECK(memchr("abcde", 'c', 5) != NULL);
    CHECK(memchr("abcde", 'z', 5) == NULL);

    strcpy(s, "foo"); strcat(s, "bar");
    CHECK_STR(s, "foobar");
    strncat(s, "bazqux", 3);
    CHECK_STR(s, "foobarbaz");

    CHECK(strcmp("abc","abc") == 0);
    CHECK(strcmp("abc","abd") < 0);
    CHECK(strncmp("abcXX","abcYY",3) == 0);

    CHECK(strchr("hello",'l') == (char*)"hello" + 2);
    CHECK(strrchr("hello",'l') == (char*)"hello" + 3);
    CHECK(strstr("hello world","world") != NULL);
    CHECK(strstr("hello world","xyz") == NULL);

    CHECK_EQ(strspn("abcxyz","abc"), 3u);
    CHECK_EQ(strcspn("abcxyz","xyz"), 3u);

    char tok[] = "one:two::three";
    char *sv;
    char *t = strtok_r(tok, ":", &sv);  CHECK_STR(t, "one");
    t = strtok_r(NULL,":", &sv);        CHECK_STR(t, "two");
    t = strtok_r(NULL,":", &sv);        CHECK_STR(t, "three");
    t = strtok_r(NULL,":", &sv);        CHECK(t == NULL);

    char rev[] = "WASM3"; strrev(rev);
    CHECK_STR(rev, "3MSAW");

    CHECK_STR(strerror(0),      "Success");
    CHECK_STR(strerror(ENOMEM), "Cannot allocate memory");
    CHECK(strerror(9999) != NULL);
}

/* ── ctype ───────────────────────────────────────────────────────────────── */

static void test_ctype(void) {
    SECTION("ctype");
    CHECK(isalpha('a') && isalpha('Z') && !isalpha('0') && !isalpha('!'));
    CHECK(isdigit('0') && isdigit('9') && !isdigit('a'));
    CHECK(isalnum('a') && isalnum('3') && !isalnum('!'));
    CHECK(isxdigit('f') && isxdigit('F') && !isxdigit('g'));
    CHECK(isspace(' ') && isspace('\t') && !isspace('a'));
    CHECK(ispunct('!') && !ispunct('a') && !ispunct(' '));
    CHECK(iscntrl('\0') && !iscntrl('a'));
    CHECK(isupper('A') && !isupper('a'));
    CHECK(islower('a') && !islower('A'));
    CHECK(isprint(' ') && !isprint('\x01'));
    CHECK_EQ(toupper('a'), 'A');
    CHECK_EQ(tolower('A'), 'a');
    CHECK_EQ(toupper('3'), '3');
    /* EOF = -1 does not crash */
    CHECK(!isalpha(-1));
}

/* ── malloc ──────────────────────────────────────────────────────────────── */

static void test_malloc(void) {
    SECTION("malloc");

    int *p = malloc(10 * sizeof(int));
    CHECK(p != NULL);
    for (int i = 0; i < 10; i++) p[i] = i*i;
    CHECK_EQ(p[9], 81);
    free(p);

    char *z = calloc(64, 1);
    int all_zero = 1;
    for (int i = 0; i < 64; i++) if (z[i]) { all_zero = 0; break; }
    CHECK(all_zero);
    free(z);

    char *r = malloc(8); strcpy(r, "hello");
    r = realloc(r, 128);
    CHECK(r && strcmp(r,"hello") == 0);
    free(r);

    void *q = realloc(NULL, 32); CHECK(q != NULL); free(q);
    void *t2 = malloc(32); t2 = realloc(t2, 0); CHECK(t2 == NULL);
    free(NULL); CHECK(1);

    /* overflow calloc */
    CHECK(calloc((size_t)-1, 2) == NULL);

    /* stress: 128 allocs, liberation in 3 passes, large re-alloc */
    void *ptrs[128];
    for (int i = 0; i < 128; i++) ptrs[i] = malloc((size_t)(i+1)*7);
    for (int i = 0; i < 128; i += 3) free(ptrs[i]);
    for (int i = 1; i < 128; i += 3) free(ptrs[i]);
    for (int i = 2; i < 128; i += 3) free(ptrs[i]);
    void *big = malloc(4096); CHECK(big != NULL); free(big);
}

/* ── stdlib ──────────────────────────────────────────────────────────────── */

static int icmp(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}
static int scmp(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

static void test_stdlib(void) {
    SECTION("stdlib");

    CHECK_EQ(atoi("42"), 42);
    CHECK_EQ(atoi("-1337"), -1337);
    CHECK_EQ(atoi("  99"), 99);
    CHECK_EQ(atoll("9876543210"), 9876543210LL);

    CHECK_EQ(strtol("0xff",  NULL, 0),  255);
    CHECK_EQ(strtol("010",   NULL, 0),  8);
    CHECK_EQ(strtol("1010",  NULL, 2),  10);

    char *end;
    long v = strtol("123abc", &end, 10);
    CHECK_EQ(v, 123); CHECK_STR(end, "abc");

    char buf[32];
    itoa(255,  buf, 16); CHECK_STR(buf, "ff");
    itoa(-42,  buf, 10); CHECK_STR(buf, "-42");
    itoa(0,    buf, 10); CHECK_STR(buf, "0");

    CHECK_EQ(abs(-99),    99);
    CHECK_EQ(labs(-1000), 1000L);
    CHECK_EQ(llabs(-1LL), 1LL);

    div_t d = div(17,5); CHECK_EQ(d.quot,3); CHECK_EQ(d.rem,2);

    srand(42);
    int r1 = rand(), r2 = rand();
    srand(42);
    CHECK_EQ(rand(), r1); CHECK_EQ(rand(), r2);
    CHECK(r1 >= 0 && r1 <= RAND_MAX);

    int nums[] = {5,3,8,1,9,2,7,4,6,0};
    qsort(nums, 10, sizeof(int), icmp);
    int sorted = 1;
    for (int i=0;i<9;i++) if (nums[i]>nums[i+1]) {sorted=0;break;}
    CHECK(sorted);
    for (int i=0;i<10;i++) CHECK_EQ(nums[i], i);

    const char *words[] = {"banana","apple","cherry","date"};
    qsort(words, 4, sizeof(char*), scmp);
    CHECK_STR(words[0],"apple"); CHECK_STR(words[3],"date");

    int key = 7;
    int *found = bsearch(&key, nums, 10, sizeof(int), icmp);
    CHECK(found && *found == 7);
    int missing = 99;
    CHECK(bsearch(&missing, nums, 10, sizeof(int), icmp) == NULL);
}

/* ── printf ──────────────────────────────────────────────────────────────── */

static void test_printf(void) {
    SECTION("printf / snprintf");
    char buf[256];

    snprintf(buf,sizeof(buf),"%d",42);          CHECK_STR(buf,"42");
    snprintf(buf,sizeof(buf),"%d",-1);          CHECK_STR(buf,"-1");
    snprintf(buf,sizeof(buf),"%05d",42);        CHECK_STR(buf,"00042");
    snprintf(buf,sizeof(buf),"%-6d|",42);       CHECK_STR(buf,"42    |");
    snprintf(buf,sizeof(buf),"%+d %+d",5,-5);   CHECK_STR(buf,"+5 -5");
    snprintf(buf,sizeof(buf),"%u",4294967295u); CHECK_STR(buf,"4294967295");
    snprintf(buf,sizeof(buf),"%x",0xDEAD);      CHECK_STR(buf,"dead");
    snprintf(buf,sizeof(buf),"%X",0xBEEF);      CHECK_STR(buf,"BEEF");
    snprintf(buf,sizeof(buf),"%#x",0xCAFE);     CHECK_STR(buf,"0xcafe");
    snprintf(buf,sizeof(buf),"%o",8);           CHECK_STR(buf,"10");
    snprintf(buf,sizeof(buf),"%b",10);          CHECK_STR(buf,"1010");
    snprintf(buf,sizeof(buf),"%lld",-9223372036854775807LL);
    CHECK_STR(buf,"-9223372036854775807");
    snprintf(buf,sizeof(buf),"%.3f",3.14159);   CHECK_STR(buf,"3.141");
    snprintf(buf,sizeof(buf),"%8.2f",3.14);     CHECK_STR(buf,"    3.14");
    snprintf(buf,sizeof(buf),"%.3s","hello");   CHECK_STR(buf,"hel");
    snprintf(buf,sizeof(buf),"%8s","hi");       CHECK_STR(buf,"      hi");
    snprintf(buf,sizeof(buf),"100%%");          CHECK_STR(buf,"100%");
    snprintf(buf,sizeof(buf),"%p",(void*)0x1234); CHECK_STR(buf,"0x00001234");
    int n = snprintf(buf,5,"hello world");
    CHECK_EQ(n,11); CHECK_STR(buf,"hell");
    int pos=0;
    snprintf(buf,sizeof(buf),"hello%n world",&pos);
    CHECK_EQ(pos,5);
    snprintf(buf,sizeof(buf),"%*d",6,42);       CHECK_STR(buf,"    42");
}

/* ── sscanf ──────────────────────────────────────────────────────────────── */

static void test_sscanf(void) {
    SECTION("sscanf");
    int i; unsigned u; float f; double d; char s[64]; char c;

    CHECK_EQ(sscanf("42",  "%d",&i),1); CHECK_EQ(i,42);
    CHECK_EQ(sscanf("-99", "%d",&i),1); CHECK_EQ(i,-99);
    CHECK_EQ(sscanf("255", "%u",&u),1); CHECK_EQ(u,255u);
    CHECK_EQ(sscanf("ff",  "%x",&u),1); CHECK_EQ(u,255u);
    CHECK_EQ(sscanf("0xFF","%x",&u),1); CHECK_EQ(u,255u);
    CHECK_EQ(sscanf("10",  "%o",&u),1); CHECK_EQ(u,8u);
    CHECK_EQ(sscanf("0x1A","%i",&i),1); CHECK_EQ(i,26);
    CHECK_EQ(sscanf("3.14","%f",&f),1); CHECK_NEAR(f,3.14f,1e-4f);
    CHECK_EQ(sscanf("3.14","%lf",&d),1);CHECK_NEAR(d,3.14,1e-10);
    CHECK_EQ(sscanf("1e2", "%lf",&d),1);CHECK_NEAR(d,100.0,1e-10);
    CHECK_EQ(sscanf("hello world","%s",s),1); CHECK_STR(s,"hello");
    CHECK_EQ(sscanf("A","%c",&c),1); CHECK_EQ(c,'A');
    CHECK_EQ(sscanf(" B","%c",&c),1); CHECK_EQ(c,' ');

    int a,b;
    CHECK_EQ(sscanf("10 20","%d %d",&a,&b),2);
    CHECK_EQ(a,10); CHECK_EQ(b,20);

    CHECK_EQ(sscanf("10 20 30","%d %*d %d",&a,&b),2);
    CHECK_EQ(a,10); CHECK_EQ(b,30);

    CHECK_EQ(sscanf("123456","%3d%3d",&a,&b),2);
    CHECK_EQ(a,123); CHECK_EQ(b,456);

    CHECK_EQ(sscanf("abc123","%[a-z]",s),1); CHECK_STR(s,"abc");
    CHECK_EQ(sscanf("123abc","%[^a-z]",s),1); CHECK_STR(s,"123");

    int np;
    sscanf("hello world","%s%n",s,&np);
    CHECK_EQ(np,5);

    CHECK_EQ(sscanf("100%%","%d%%",&i),1); CHECK_EQ(i,100);
    CHECK_EQ(sscanf("","  %d",&i),-1);
    CHECK_EQ(sscanf("abc","%d",&i),-1);
}

/* ── math ────────────────────────────────────────────────────────────────── */

static void test_math(void) {
    SECTION("math");
#define PI 3.14159265358979323846
#define EE 2.71828182845904523536
#define T  1e-9

    CHECK(isnan(NAN) && !isnan(1.0));
    CHECK(isinf(INFINITY) && !isinf(1.0));
    CHECK(isfinite(1.0) && !isfinite(INFINITY));

    CHECK_NEAR(fabs(-3.14), 3.14, 0.0);
    CHECK_NEAR(floor(1.9),  1.0,  0.0);
    CHECK_NEAR(floor(-1.1),-2.0,  0.0);
    CHECK_NEAR(ceil(1.1),   2.0,  0.0);
    CHECK_NEAR(round(1.5),  2.0,  0.0);
    CHECK_NEAR(trunc(1.9),  1.0,  0.0);
    CHECK_NEAR(fmod(10.0,3.0),1.0,T);

    CHECK_NEAR(sqrt(4.0), 2.0, T);
    CHECK_NEAR(sqrt(2.0), 1.41421356237, T);
    CHECK(isnan(sqrt(-1.0)));
    CHECK_NEAR(cbrt(8.0),  2.0, T);
    CHECK_NEAR(cbrt(-27.0),-3.0,T);

    CHECK_NEAR(exp(0.0), 1.0,  T);
    CHECK_NEAR(exp(1.0), EE,   T);
    CHECK_NEAR(log(1.0), 0.0,  T);
    CHECK_NEAR(log(EE),  1.0,  T);
    CHECK_NEAR(log2(8.0),3.0,  T);
    CHECK_NEAR(log10(1000.0),3.0,T);
    CHECK(isinf(log(0.0)) && log(0.0) < 0);
    CHECK(isnan(log(-1.0)));

    CHECK_NEAR(pow(2.0,10.0), 1024.0, T);
    CHECK_NEAR(pow(9.0, 0.5), 3.0,    T);
    CHECK_EQ  (pow(2.0, 0.0), 1.0);

    CHECK_NEAR(sin(0.0),    0.0, T);
    CHECK_NEAR(sin(PI/2),   1.0, T);
    CHECK_NEAR(sin(-PI/2), -1.0, T);
    CHECK_NEAR(cos(0.0),    1.0, T);
    CHECK_NEAR(cos(PI),    -1.0, T);
    CHECK_NEAR(tan(PI/4),   1.0, T);

    /* fundamental identity sin²+cos²=1 */
    double ang = 1.23456;
    CHECK_NEAR(sin(ang)*sin(ang)+cos(ang)*cos(ang), 1.0, 1e-14);

    CHECK_NEAR(asin(1.0),  PI/2, T);
    CHECK_NEAR(acos(1.0),  0.0,  T);
    CHECK_NEAR(atan(1.0),  PI/4, T);
    CHECK_NEAR(atan2(0.0,-1.0), PI, T);

    CHECK_NEAR(sinh(0.0), 0.0, T);
    CHECK_NEAR(cosh(0.0), 1.0, T);
    CHECK_NEAR(tanh(100.0),1.0,T);
    /* cosh²-sinh²=1 */
    double x=1.5;
    CHECK_NEAR(cosh(x)*cosh(x)-sinh(x)*sinh(x), 1.0, 1e-13);

    CHECK_NEAR(hypot(3.0,4.0), 5.0, T);
    CHECK_NEAR(sinf(0.0f), 0.0f, 1e-6f);
    CHECK_NEAR(sqrtf(4.0f),2.0f, 1e-6f);

#undef PI
#undef EE
#undef T
}

/* ── locale ──────────────────────────────────────────────────────────────── */

static void test_locale(void) {
    SECTION("locale");
    CHECK_STR(setlocale(LC_ALL,"C"), "C");
    CHECK_STR(setlocale(LC_ALL,"POSIX"),"C");
    CHECK(setlocale(LC_ALL,NULL) != NULL);
    CHECK(setlocale(LC_ALL,"pt_BR") == NULL);
    struct lconv *lc = localeconv();
    CHECK(lc != NULL);
    CHECK_STR(lc->decimal_point,".");
}

/* ── time ────────────────────────────────────────────────────────────────── */

static void test_time(void) {
    SECTION("time (calendar)");
    struct tm t = {0};

    /* epoch zero = 1970-01-01 Thursday */
    time_t zero = 0;
    struct tm *g = gmtime(&zero);
    CHECK_EQ(g->tm_year, 70);
    CHECK_EQ(g->tm_mday, 1);
    CHECK_EQ(g->tm_wday, 4);

    /* round-trip mktime → gmtime */
    t.tm_year=124; t.tm_mon=0; t.tm_mday=15;
    t.tm_hour=12;  t.tm_min=30; t.tm_sec=45;
    time_t ep = mktime(&t);
    g = gmtime(&ep);
    CHECK_EQ(g->tm_year,124); CHECK_EQ(g->tm_mday,15);
    CHECK_EQ(g->tm_hour,12);  CHECK_EQ(g->tm_min,30);

    /* Y2K Saturday */
    t=(struct tm){0}; t.tm_year=100; t.tm_mon=0; t.tm_mday=1;
    ep=mktime(&t); g=gmtime(&ep);
    CHECK_EQ(g->tm_wday, 6);

    /* 2000-02-29 valid (leap year) */
    t=(struct tm){0}; t.tm_year=100; t.tm_mon=1; t.tm_mday=29;
    ep=mktime(&t); g=gmtime(&ep);
    CHECK_EQ(g->tm_mday,29); CHECK_EQ(g->tm_mon,1);

    CHECK_NEAR(difftime(100,60), 40.0, 0.0);

    /* strftime */
    char buf[128];
    t=(struct tm){0};
    t.tm_year=124; t.tm_mon=0; t.tm_mday=15;
    t.tm_hour=9;   t.tm_min=5;  t.tm_sec=3; t.tm_wday=1;
    strftime(buf,sizeof(buf),"%Y-%m-%d",&t); CHECK_STR(buf,"2024-01-15");
    strftime(buf,sizeof(buf),"%H:%M:%S",&t); CHECK_STR(buf,"09:05:03");
    strftime(buf,sizeof(buf),"100%%",&t);    CHECK_STR(buf,"100%");
}

/* ── errno ───────────────────────────────────────────────────────────────── */

static void test_errno(void) {
    SECTION("errno");
    errno = EINVAL;
    CHECK_EQ(errno, EINVAL);
    CHECK_STR(strerror(EINVAL),"Invalid argument");
    errno = 0;
}

/* ── assert ──────────────────────────────────────────────────────────────── */

static void test_assert_mod(void) {
    SECTION("assert");
    assert(1 == 1);
    assert(42 > 0);
    static_assert(sizeof(int) == 4, "int = 4 bytes in wasm32");
    static_assert(sizeof(long long) == 8, "llong = 8 bytes");
    CHECK(1);
}

/* ── main ────────────────────────────────────────────────────────────────── */

int wasm_main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("wasm-libc test suite v3\n");
    printf("=======================\n");

    test_string();
    test_ctype();
    test_malloc();
    test_stdlib();
    test_printf();
    test_sscanf();
    test_math();
    test_locale();
    test_time();
    test_errno();
    test_assert_mod();

    printf("\n=======================\n");
    printf("result: %d/%d passed", _pass, _total);
    if (_fail) printf("  (%d FAILED)", _fail);
    printf("\n");
    return _fail ? 1 : 0;
}

void _start(void) {
    wasm_main(0, 0);
}
