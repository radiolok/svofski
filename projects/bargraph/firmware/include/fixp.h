//
// Fixed point routines courtesy of Felix Lazarev
//
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FIXP_WBITS      16

typedef int fixp;
typedef long long       fixpd;
typedef unsigned int fixpu;
typedef unsigned long long fixpud;

#define FIXP_FBITS      (32 - FIXP_WBITS)
#define FIXP_FMASK      (((fixp)1 << FIXP_FBITS) - 1)

//#define fixp_rconst(R) ((fixp)((R) * FIXP_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define fixp_rconst(R) ((fixp)((R) * FIXP_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define fixp_fromint(I) ((fixpd)(I) << FIXP_FBITS)
#define fixp_toint(F) ((F) >> FIXP_FBITS)
#define fixp_add(A,B) ((A) + (B))
#define fixp_sub(A,B) ((A) - (B))
#define fixp_xmul(A,B)                                          \
    ((fixp)(((fixpd)(A) * (fixpd)(B)) >> FIXP_FBITS))
#define fixp_xdiv(A,B)                                          \
    ((fixp)(((fixpd)(A) << FIXP_FBITS) / (fixpd)(B)))
#define fixp_fracpart(A) ((fixp)(A) & FIXP_FMASK)
#define FIXP_ONE        ((fixp)((fixp)1 << FIXP_FBITS))
#define FIXP_ONE_HALF (FIXP_ONE >> 1)
#define FIXP_TWO        (FIXP_ONE + FIXP_ONE)
#define FIXP_PI fixp_rconst(3.14159265358979323846)
#define FIXP_TWO_PI     fixp_rconst(2 * 3.14159265358979323846)
#define FIXP_HALF_PI    fixp_rconst(3.14159265358979323846 / 2)
#define FIXP_E  fixp_rconst(2.7182818284590452354)
#define fixp_abs(A) ((A) < 0 ? -(A) : (A))


fixp fixp_mul(fixp A, fixp B);
fixp fixp_div(fixp A, fixp B);
fixp fixp_ln(fixp x);
fixp fixp_log(fixp x, fixp base);
fixp fixp_test(fixp x);


#ifdef __cplusplus
}
#endif

