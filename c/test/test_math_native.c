/*
 * test_math_native.c — validates math.c against system libm
 *
 * Compiles with pure gcc (no -ffreestanding) using WASM builtin stubs.
 * Runs as a native binary to detect bugs in algorithms before
 * compiling to WASM.
 *
 * Usage:
 *   gcc -O2 -std=c11 -I../include -o test_math_native test_math_native.c \
 *       ../src/math/math.c -lm
 *   ./test_math_native
 */

#include <stdio.h>
#include <math.h>    /* system libm — for comparison */
#include <stdint.h>
#include <string.h>

/* includes our implementation with a different name via wrapper */
/* renames system functions to avoid conflict */
#define sin   _sys_sin
#define cos   _sys_cos
#define tan   _sys_tan
#define asin  _sys_asin
#define acos  _sys_acos
#define atan  _sys_atan
#define atan2 _sys_atan2
#define exp   _sys_exp
#define log   _sys_log
#define log2  _sys_log2
#define log10 _sys_log10
#define pow   _sys_pow
#define sqrt  _sys_sqrt
#define cbrt  _sys_cbrt
#define sinh  _sys_sinh
#define cosh  _sys_cosh
#define tanh  _sys_tanh
#define floor _sys_floor
#define ceil  _sys_ceil
#define round _sys_round
#define trunc _sys_trunc
#define fmod  _sys_fmod
#define fabs  _sys_fabs
#include <math.h>
#undef sin
#undef cos
#undef tan
#undef asin
#undef acos
#undef atan
#undef atan2
#undef exp
#undef log
#undef log2
#undef log10
#undef pow
#undef sqrt
#undef cbrt
#undef sinh
#undef cosh
#undef tanh
#undef floor
#undef ceil
#undef round
#undef trunc
#undef fmod
#undef fabs

/* now includes our implementation with real names */
#include "../../include/math.h"

/* wasm builtin stubs for native compilation */
double __builtin_huge_val(void)          { return 1e308 * 1e308; }
float  __builtin_inff(void)              { return (float)(1e38f * 1e38f); }
float  __builtin_nanf(const char *s)     { (void)s; return 0.0f/0.0f; }

/* ── framework ───────────────────────────────────────────────────────────── */

static int pass = 0, fail = 0;

static void check_d(const char *name, double got, double expected, double tol) {
    double err = got - expected;
    if (err < 0) err = -err;
    double ref = expected < 0 ? -expected : expected;
    double rel = ref > 1e-15 ? err / ref : err;
    if (rel <= tol) {
        printf("  OK   %-28s got=%.15g exp=%.15g\n", name, got, expected);
        pass++;
    } else {
        printf("  FAIL %-28s got=%.15g exp=%.15g err=%.2e\n",
               name, got, expected, rel);
        fail++;
    }
}

#define EPS 1e-12
#define EPS_F 1e-6

#define CK(fn, arg, sys_fn) \
    check_d(#fn "(" #arg ")", fn(arg), sys_fn(arg), EPS)

#define CK2(fn, a, b, sys_fn) \
    check_d(#fn "(" #a "," #b ")", fn(a,b), sys_fn(a,b), EPS)

/* ── testes ──────────────────────────────────────────────────────────────── */

static void test_trig(void) {
    printf("\n[trigonometry]\n");
    double angles[] = { 0.0, 0.1, 0.5, 1.0, M_PI/6, M_PI/4, M_PI/3,
                        M_PI/2, M_PI, 2*M_PI, -1.5, 3.7 };
    int n = (int)(sizeof(angles)/sizeof(angles[0]));
    char name[64];
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "sin(%.4g)", angles[i]);
        check_d(name, sin(angles[i]), _sys_sin(angles[i]), EPS);
        snprintf(name, sizeof(name), "cos(%.4g)", angles[i]);
        check_d(name, cos(angles[i]), _sys_cos(angles[i]), EPS);
        if (angles[i] != M_PI/2 && angles[i] != -M_PI/2) {
            snprintf(name, sizeof(name), "tan(%.4g)", angles[i]);
            check_d(name, tan(angles[i]), _sys_tan(angles[i]), 1e-11);
        }
    }
    double avals[] = { 0.0, 0.1, 0.5, 0.9, 1.0, -0.5, -1.0 };
    int an = (int)(sizeof(avals)/sizeof(avals[0]));
    for (int i = 0; i < an; i++) {
        snprintf(name, sizeof(name), "asin(%.4g)", avals[i]);
        check_d(name, asin(avals[i]), _sys_asin(avals[i]), EPS);
        snprintf(name, sizeof(name), "acos(%.4g)", avals[i]);
        check_d(name, acos(avals[i]), _sys_acos(avals[i]), EPS);
    }
    double atvals[] = { 0.0, 0.5, 1.0, 2.0, 10.0, -1.0, -5.0 };
    int atn = (int)(sizeof(atvals)/sizeof(atvals[0]));
    for (int i = 0; i < atn; i++) {
        snprintf(name, sizeof(name), "atan(%.4g)", atvals[i]);
        check_d(name, atan(atvals[i]), _sys_atan(atvals[i]), EPS);
    }
    check_d("atan2(1,1)",  atan2(1.0,1.0),  _sys_atan2(1.0,1.0),  EPS);
    check_d("atan2(-1,1)", atan2(-1.0,1.0), _sys_atan2(-1.0,1.0), EPS);
    check_d("atan2(1,-1)", atan2(1.0,-1.0), _sys_atan2(1.0,-1.0), EPS);
    check_d("atan2(0,0)",  atan2(0.0,0.0),  _sys_atan2(0.0,0.0),  EPS);
}

static void test_exp_log(void) {
    printf("\n[exp / log]\n");
    double evals[] = { 0.0, 0.1, 0.5, 1.0, 2.0, 10.0, -1.0, -10.0, 100.0, 700.0 };
    int n = (int)(sizeof(evals)/sizeof(evals[0]));
    char name[64];
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "exp(%.4g)", evals[i]);
        check_d(name, exp(evals[i]), _sys_exp(evals[i]), 1e-11);
    }
    double lvals[] = { 1.0, M_E, 2.0, 10.0, 100.0, 0.5, 1e-5, 1e10 };
    int ln = (int)(sizeof(lvals)/sizeof(lvals[0]));
    for (int i = 0; i < ln; i++) {
        snprintf(name, sizeof(name), "log(%.4g)", lvals[i]);
        check_d(name, log(lvals[i]), _sys_log(lvals[i]), EPS);
        snprintf(name, sizeof(name), "log2(%.4g)", lvals[i]);
        check_d(name, log2(lvals[i]), _sys_log2(lvals[i]), EPS);
        snprintf(name, sizeof(name), "log10(%.4g)", lvals[i]);
        check_d(name, log10(lvals[i]), _sys_log10(lvals[i]), EPS);
    }
}

static void test_pow_sqrt(void) {
    printf("\n[pow / sqrt / cbrt]\n");
    check_d("sqrt(2)",    sqrt(2.0),    _sys_sqrt(2.0),    EPS);
    check_d("sqrt(9)",    sqrt(9.0),    _sys_sqrt(9.0),    EPS);
    check_d("sqrt(0.5)",  sqrt(0.5),    _sys_sqrt(0.5),    EPS);
    check_d("sqrt(1e10)", sqrt(1e10),   _sys_sqrt(1e10),   EPS);
    check_d("cbrt(8)",    cbrt(8.0),    _sys_cbrt(8.0),    EPS);
    check_d("cbrt(-27)",  cbrt(-27.0),  _sys_cbrt(-27.0),  EPS);
    check_d("cbrt(2)",    cbrt(2.0),    _sys_cbrt(2.0),    EPS);
    check_d("pow(2,10)",  pow(2.0,10.0),_sys_pow(2.0,10.0),EPS);
    check_d("pow(2,-3)",  pow(2.0,-3.0),_sys_pow(2.0,-3.0),EPS);
    check_d("pow(e,pi)",  pow(M_E,M_PI),_sys_pow(M_E,M_PI),EPS);
    check_d("pow(9,0.5)", pow(9.0,0.5), _sys_pow(9.0,0.5), EPS);
    check_d("pow(2,0.333)",pow(2.0,1.0/3.0),_sys_pow(2.0,1.0/3.0),1e-10);
}

static void test_hyperbolic(void) {
    printf("\n[hyperbolic]\n");
    double vals[] = { 0.0, 0.1, 0.5, 1.0, 2.0, 5.0, -1.0, -3.0 };
    int n = (int)(sizeof(vals)/sizeof(vals[0]));
    char name[64];
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "sinh(%.4g)", vals[i]);
        check_d(name, sinh(vals[i]), _sys_sinh(vals[i]), 1e-10);
        snprintf(name, sizeof(name), "cosh(%.4g)", vals[i]);
        check_d(name, cosh(vals[i]), _sys_cosh(vals[i]), 1e-10);
        snprintf(name, sizeof(name), "tanh(%.4g)", vals[i]);
        check_d(name, tanh(vals[i]), _sys_tanh(vals[i]), EPS);
    }
}

static void test_rounding(void) {
    printf("\n[rounding]\n");
    double vals[] = { 1.5, 1.4, -1.5, -1.4, 2.5, -2.5, 0.0, 100.9 };
    int n = (int)(sizeof(vals)/sizeof(vals[0]));
    char name[64];
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "floor(%.2f)", vals[i]);
        check_d(name, floor(vals[i]), _sys_floor(vals[i]), 0.0);
        snprintf(name, sizeof(name), "ceil(%.2f)",  vals[i]);
        check_d(name, ceil(vals[i]),  _sys_ceil(vals[i]),  0.0);
        snprintf(name, sizeof(name), "round(%.2f)", vals[i]);
        check_d(name, round(vals[i]), _sys_round(vals[i]), 0.0);
        snprintf(name, sizeof(name), "trunc(%.2f)", vals[i]);
        check_d(name, trunc(vals[i]), _sys_trunc(vals[i]), 0.0);
    }
    check_d("fmod(10,3)",  fmod(10.0,3.0),  _sys_fmod(10.0,3.0),  EPS);
    check_d("fmod(-7,3)",  fmod(-7.0,3.0),  _sys_fmod(-7.0,3.0),  EPS);
    check_d("fmod(5.5,2)", fmod(5.5,2.0),   _sys_fmod(5.5,2.0),   EPS);
    check_d("fabs(-3.14)", fabs(-3.14),      _sys_fabs(-3.14),     0.0);
    check_d("fabs(0)",     fabs(0.0),        _sys_fabs(0.0),       0.0);
}

int main(void) {
    printf("wasm-libc math validation (native)\n");
    printf("====================================\n");

    test_trig();
    test_exp_log();
    test_pow_sqrt();
    test_hyperbolic();
    test_rounding();

    printf("\n====================================\n");
    printf("result: %d/%d tests passed\n", pass, pass + fail);
    return fail > 0 ? 1 : 0;
}
