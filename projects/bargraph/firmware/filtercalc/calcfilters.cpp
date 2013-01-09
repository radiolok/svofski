
#include <stdio.h>
#include <math.h>

#include "../include/biquad2.h"

#include "frequencies.h"

int main(int argc, char **argv) {
	int sampleRate = SAMPLERATE;
    char buf[1024];

    FILE* out = fopen("instfilters.h", "w");

    float Q = 10;//8;

	fprintf(out, "Biquad f1 = %s;\n", Biquad(sampleRate, Freq[0]*2,  Q,  1).toString(buf));
	fprintf(out, "Biquad f2 = %s;\n", Biquad(sampleRate, Freq[1]*2,  Q,  1).toString(buf));
	fprintf(out, "Biquad f3 = %s;\n", Biquad(sampleRate, Freq[2],    Q,  0).toString(buf));
	fprintf(out, "Biquad f4 = %s;\n", Biquad(sampleRate, Freq[3],    Q,  0).toString(buf));
	fprintf(out, "Biquad f5 = %s;\n", Biquad(sampleRate, Freq[4],    Q,  0).toString(buf));
	fprintf(out, "Biquad f6 = %s;\n", Biquad(sampleRate, Freq[5],    Q,  0).toString(buf));
	fprintf(out, "Biquad f7 = %s;\n", Biquad(sampleRate, Freq[6],    Q,  0).toString(buf));
	fprintf(out, "Biquad f8 = %s;\n", Biquad(sampleRate, Freq[7],    Q,  0).toString(buf));
	fprintf(out, "Biquad f9 = %s;\n", Biquad(sampleRate, Freq[8],    Q,  0).toString(buf));
	fprintf(out, "Biquad f10 = %s;\n", Biquad(sampleRate, Freq[9],   Q,  0).toString(buf));
	fprintf(out, "Biquad f11 = %s;\n", Biquad(sampleRate, Freq[10],  Q,  0).toString(buf));
	fprintf(out, "Biquad f12 = %s;\n", Biquad(sampleRate, Freq[11],  Q,  0).toString(buf));
	fprintf(out, "Biquad f13 = %s;\n", Biquad(sampleRate, Freq[12],  Q,  0).toString(buf));
	fprintf(out, "Biquad f14 = %s;\n", Biquad(sampleRate, Freq[13],  Q,  0).toString(buf));
	fprintf(out, "Biquad f15 = %s;\n", Biquad(sampleRate, Freq[14],  Q,  0).toString(buf));
	fprintf(out, "Biquad f16 = %s;\n", Biquad(sampleRate, Freq[15],  Q,  0).toString(buf));
	fprintf(out, "Biquad f17 = %s;\n", Biquad(sampleRate, Freq[16],  Q,  0).toString(buf));
	fprintf(out, "Biquad f18 = %s;\n", Biquad(sampleRate, Freq[17],  Q,  0).toString(buf));
	fprintf(out, "Biquad f19 = %s;\n", Biquad(sampleRate, Freq[18],  Q,0).toString(buf));
	fprintf(out, "Biquad f20 = %s;\n", Biquad(sampleRate, Freq[19],  Q,0).toString(buf));
    fclose(out);

	return 0;
}
