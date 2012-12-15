//
// Fixed point routines courtesy of Felix Lazarev
//
#include "fixp.h"

fixp fixp_mul(fixp A, fixp B)
{
    return (((fixpd)A * (fixpd)B) >> FIXP_FBITS);
}

fixp fixp_div(fixp A, fixp B)
{
    return (((fixpd)A << FIXP_FBITS) / (fixpd)B);
}

fixp fixp_ln(fixp x)
{
    fixp log2, xi;
    fixp f, s, z, w, R;

    const fixp LG[7] = {
            fixp_rconst(6.666666666666735130e-01),
            fixp_rconst(3.999999999940941908e-01),
            fixp_rconst(2.857142874366239149e-01),
            fixp_rconst(2.222219843214978396e-01),
            fixp_rconst(1.818357216161805012e-01),
            fixp_rconst(1.531383769920937332e-01),
            fixp_rconst(1.479819860511658591e-01)
    };

    const fixp LN2 = fixp_rconst(0.69314718055994530942);


    if (x < 0)
            return 0;
    if (x == 0)
            return 0xffffffff;
    log2 = 0;
    xi = x;
    while (xi > FIXP_TWO) {
            xi >>= 1;
            log2++;
    }
    f = xi - FIXP_ONE;
    s = fixp_div(f, FIXP_TWO + f);
    z = fixp_mul(s, s);
    w = fixp_mul(z, z);
    R = fixp_mul(w, LG[1] + fixp_mul(w, LG[3]
       + fixp_mul(w, LG[5]))) + fixp_mul(z, LG[0]
       + fixp_mul(w, LG[2] + fixp_mul(w, LG[4]
       + fixp_mul(w, LG[6]))));
    return (fixp_mul(LN2, (log2 << FIXP_FBITS)) + f
        - fixp_mul(s, f - R));
}
fixp fixp_log(fixp x, fixp base)
{
    return (fixp_div(fixp_ln(x), fixp_ln(base)));
}

fixp fixp_test(fixp x) {
    if (x < 0)
        return 0;
    return x;
}

