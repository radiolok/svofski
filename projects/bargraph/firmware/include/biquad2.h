/*
 * With thanks to Nigel Redmon http://www.earlevel.com/main/2011/01/02/biquad-formulas/
 */
#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fixp.h"

class Biquad {
public:
    Biquad(int a0, int a1, int a2, int b1, int b2, int freq, int decimate
#ifdef TESTBENCH
        , float q, float fa0, float fa1, float fa2, float fb1, float fb2
#endif
    ) : m_Decimate(decimate) {
        m_ia0 = a0;
        m_ia1 = a1;
        m_ia2 = a2;
        m_ib1 = b1;
        m_ib2 = b2;
        m_ix_1 = m_ix_2 = m_iy_1 = m_iy_2 = 0;
        m_id_1 = m_id_2 = 0;
        m_Freq = freq;
#ifdef TESTBENCH
        m_Q = q;
        m_a0 = fa0;
        m_a1 = fa1;
        m_a2 = fa2;
        m_b1 = fb1;
        m_b2 = fb2;
#endif
    }

    int IA0() const { return m_ia0; };
    int IA1() const { return m_ia1; };
    int IA2() const { return m_ia2; };
    int IB1() const { return m_ib1; };
    int IB2() const { return m_ib2; };
    int freq() const { return m_Freq; };
    inline int decimated() const { return m_Decimate; }

#ifdef TESTBENCH
    Biquad(int sampleRate, float freq, float Q, int decimate);
    float ffilter(float x) {
        float result = m_a0*x + m_a1*m_x_1 + m_a2*m_x_2 - m_b1*m_y_1 - m_b2*m_y_2;
        m_x_2 = m_x_1;
        m_x_1 = x;
        m_y_2 = m_y_1;
        m_y_1 = result;
        return result;
    }
    float Q() const { return m_Q; };
    float A0() const { return m_a0; };
    float A1() const { return m_a1; };
    float A2() const { return m_a2; };
    float B1() const { return m_b1; };
    float B2() const { return m_b2; };

    char* toString(char* buf) {
        sprintf(buf, "Biquad(%d,%d,%d,%d,%d, %d, %d\n#ifdef TESTBENCH\n ,%f,%f,%f,%f,%f,%f\n#endif\n)",
            m_ia0, m_ia1, m_ia2, m_ib1, m_ib2, m_Freq, m_Decimate,
            m_Q, m_a0, m_a1, m_a2, m_b1, m_b2);
        return buf;
    }
#endif

    inline int ifilter(int x) {
#ifdef FORM_I
        int result = fixp_xmul(m_ia0,x) 
            + fixp_xmul(m_ia1,m_ix_1)
            + fixp_xmul(m_ia2,m_ix_2)
            - fixp_xmul(m_ib1,m_iy_1)
            - fixp_xmul(m_ib2,m_iy_2);

        m_ix_2 = m_ix_1;
        m_ix_1 = x;
        m_iy_2 = m_iy_1;
        m_iy_1 = result;
#else
        fixp dn = x - fixp_xmul(m_ib1, m_id_1) - fixp_xmul(m_ib2, m_id_2);
        fixp result = fixp_xmul(m_ia0, dn) + fixp_xmul(m_ia1, m_id_1) + fixp_xmul(m_ia2, m_id_2);

        m_id_2 = m_id_1;
        m_id_1 = dn;
#endif
        return result;
    }

    int scale() const { return 32768; }


private:
#ifdef TESTBENCH
    void calcBandpass(int sampleRate, float freq, float Q);
    void calcInteger();
    float m_Q;

    float m_a0, m_a1, m_a2, m_b1, m_b2;
    float m_x_1, m_x_2, m_y_1, m_y_2;

#endif
    int m_Freq;
    int m_ia0, m_ia1, m_ia2, m_ib1, m_ib2;
    int m_ix_1, m_ix_2, m_iy_1, m_iy_2;
    int m_id_1, m_id_2;
    const int m_Decimate;
};

#ifdef TESTBENCH
Biquad::Biquad(int sampleRate, float freq, float Q, int decimate) : m_Decimate(decimate) {
    m_Freq = freq;
    m_Q = Q;
    calcBandpass(sampleRate, freq, m_Q);
    calcInteger();
    //printf("bandpass: q=%f fs=%f %d %d %d %d %d\n", m_Q, m_Freq, m_ia0, m_ia1, m_ia2, m_ib1, m_ib2);
    m_x_1 = m_x_2 = m_y_1 = m_y_2 = 0;
    m_ix_1 = m_ix_2 = m_iy_1 = m_iy_2 = 0;
    m_id_1 = m_id_2 = 0;
}

void Biquad::calcBandpass(int sampleRate, float freq, float Q) {
    float norm;
    float V, K;

    K = tan(M_PI * freq/(2*sampleRate));
    norm = 1 / (1 + K / Q + K * K);
    m_a0 = K / Q * norm;
    m_a1 = 0;
    m_a2 = -m_a0;
    m_b1 = 2 * (K * K - 1) * norm;
    m_b2 = (1 - K / Q + K * K) * norm;
}

void Biquad::calcInteger() {
    m_ia0 = fixp_rconst(m_a0);
    m_ia1 = fixp_rconst(m_a1);
    m_ia2 = fixp_rconst(m_a2);
    m_ib1 = fixp_rconst(m_b1);
    m_ib2 = fixp_rconst(m_b2);
}
#endif

#ifndef TESTBENCH

#include "instfilters.h"

#endif
