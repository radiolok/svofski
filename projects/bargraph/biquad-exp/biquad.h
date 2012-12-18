/*
 * With thanks to Nigel Redmon http://www.earlevel.com/main/2011/01/02/biquad-formulas/
 */
#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define FIXP_WBITS      16
 
typedef int fixp;
typedef long long       fixpd;
typedef unsigned int fixpu;
typedef unsigned long long fixpud;

#define FIXP_FBITS      (32 - FIXP_WBITS)
#define FIXP_FMASK      (((fixp)1 << FIXP_FBITS) - 1)

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
 
 
static inline fixp fixp_mul(fixp A, fixp B)
{
    return (((fixpd)A * (fixpd)B) >> FIXP_FBITS);
}
 
static inline fixp fixp_div(fixp A, fixp B)
{
    return (((fixpd)A << FIXP_FBITS) / (fixpd)B);
}

class Filter {
public:
	virtual float ffilter(float x) = 0;
	virtual int ifilter(int32_t x) = 0;
	virtual int32_t scale() const = 0;
	virtual float freq() const = 0;
	virtual float Q() const = 0;
	virtual int sampleRate() const = 0;
	virtual float A0() const = 0;
	virtual float A1() const = 0;
	virtual float A2() const = 0;
	virtual float B1() const = 0;
	virtual float B2() const = 0;
};

template <size_t S> class Biquad: public Filter {
public:
	Biquad(int sampleRate, float freq, float Q);

	float A0() const { return m_a0; };
	float A1() const { return m_a1; };
	float A2() const { return m_a2; };
	float B1() const { return m_b1; };
	float B2() const { return m_b2; };

	int32_t IA0() const { return m_ia0; };
	int32_t IA1() const { return m_ia1; };
	int32_t IA2() const { return m_ia2; };
	int32_t IB1() const { return m_ib1; };
	int32_t IB2() const { return m_ib2; };

    char* toString(char* buf) {
        sprintf(buf, "Biquad(%d,%d,%d,%d,%d)",
            m_ia0, m_ia1, m_ia2, m_ib1, m_ib2);
        return buf;
    }

	float ffilter(float x) {
		float result = m_a0*x + m_a1*m_x_1 + m_a2*m_x_2 - m_b1*m_y_1 - m_b2*m_y_2;
		m_x_2 = m_x_1;
		m_x_1 = x;
		m_y_2 = m_y_1;
		m_y_1 = result;
		return result;
	}

	int ifilter(int32_t x) {
		fixp q0 = fixp_mul(m_ia0,x); 
		fixp q1 = fixp_mul(m_ia1,m_ix_1);
		fixp q2 = fixp_mul(m_ia2,m_ix_2);

        fixp q3 = fixp_mul(m_ib1,m_iy_1);
        fixp q4 = fixp_mul(m_ib2,m_iy_2);
        
        int result = (q0 + q1 + q2);
        result = result - (q3 + q4);

		m_ix_2 = m_ix_1;
		m_ix_1 = x;
		m_iy_2 = m_iy_1;
		m_iy_1 = result;
		return result;
	}

	int32_t scale() const { return (int32_t) S; }
	float freq() const { return m_Freq; };
	float Q() const { return m_Q; };
	int32_t sampleRate() const { return m_SampleRate; };


private:
	void calcBandpass(int sampleRate, float freq, float Q);
	void calcInteger();

	float m_Freq;
	float m_SampleRate;
	float m_Q;

	float m_a0, m_a1, m_a2, m_b1, m_b2;
	float m_x_1, m_x_2, m_y_1, m_y_2;

	int32_t m_ia0, m_ia1, m_ia2, m_ib1, m_ib2;
	int32_t m_ix_1, m_ix_2, m_iy_1, m_iy_2;
    int32_t m_d_0, m_d_1, m_d_2;
};


template <size_t S> Biquad<S>::Biquad(int sampleRate, float freq, float Q) {
	m_Freq = freq;
	m_SampleRate = sampleRate;
	m_Q = Q;
	calcBandpass(sampleRate, freq, m_Q);
	calcInteger();
    printf("1 = %d\n", FIXP_ONE);
    printf("bandpass: q=%f fs=%f %f %f %f %f %f\n", m_Q, m_Freq, m_a0, m_a1, m_a2, m_b1, m_b2);
    printf("bandpass: q=%f fs=%f %d %d %d %d %d\n", m_Q, m_Freq, m_ia0, m_ia1, m_ia2, m_ib1, m_ib2);
	m_x_1 = m_x_2 = m_y_1 = m_y_2 = 0;
	m_ix_1 = m_ix_2 = m_iy_1 = m_iy_2 = 0;
    m_d_0 = m_d_1 = m_d_2 = 0;
}

template <size_t S> void Biquad<S>::calcBandpass(int sampleRate, float freq, float Q) {
	float norm;
	float V, K;

	K = tan(M_PI * freq/sampleRate);
	norm = 1 / (1 + K / Q + K * K);
	m_a0 = K / Q * norm;
	m_a1 = 0;
	m_a2 = -m_a0;
	m_b1 = 2 * (K * K - 1) * norm;
	m_b2 = (1 - K / Q + K * K) * norm;
}

template <size_t S> void Biquad<S>::calcInteger() {
	m_ia0 = fixp_rconst(m_a0);
	m_ia1 = fixp_rconst(m_a1);
	m_ia2 = fixp_rconst(m_a2);
	m_ib1 = fixp_rconst(m_b1);
	m_ib2 = fixp_rconst(m_b2);
}
