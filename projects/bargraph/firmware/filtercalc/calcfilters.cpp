
#include <stdio.h>
#include <math.h>

#include "../include/biquad2.h"

int main(int argc, char **argv) {
	int sampleRate = SAMPLERATE;
    char buf[1024];

    FILE* out = fopen("instfilters.h", "w");

    float Q = 10;

	fprintf(out, "Biquad f1 = %s;\n", Biquad(sampleRate, 44,       Q).toString(buf));
	fprintf(out, "Biquad f2 = %s;\n", Biquad(sampleRate, 60.0,     Q).toString(buf));
	fprintf(out, "Biquad f3 = %s;\n", Biquad(sampleRate, 80.0,     Q).toString(buf));
	fprintf(out, "Biquad f4 = %s;\n", Biquad(sampleRate, 107.0,    Q).toString(buf));
	fprintf(out, "Biquad f5 = %s;\n", Biquad(sampleRate, 145.0,    Q).toString(buf));
	fprintf(out, "Biquad f6 = %s;\n", Biquad(sampleRate, 195.0,    Q).toString(buf));
	fprintf(out, "Biquad f7 = %s;\n", Biquad(sampleRate, 264.0,    Q).toString(buf));
	fprintf(out, "Biquad f8 = %s;\n", Biquad(sampleRate, 356.0,    Q).toString(buf));
	fprintf(out, "Biquad f9 = %s;\n", Biquad(sampleRate, 480.0,    Q).toString(buf));
	fprintf(out, "Biquad f10 = %s;\n", Biquad(sampleRate, 647.0,   Q).toString(buf));
	fprintf(out, "Biquad f11 = %s;\n", Biquad(sampleRate, 872.0,   Q).toString(buf));
	fprintf(out, "Biquad f12 = %s;\n", Biquad(sampleRate, 1175.0,  Q).toString(buf));
	fprintf(out, "Biquad f13 = %s;\n", Biquad(sampleRate, 1584.0,  Q).toString(buf));
	fprintf(out, "Biquad f14 = %s;\n", Biquad(sampleRate, 2135.0,  Q).toString(buf));
	fprintf(out, "Biquad f15 = %s;\n", Biquad(sampleRate, 2878.0,  Q).toString(buf));
	fprintf(out, "Biquad f16 = %s;\n", Biquad(sampleRate, 3880.0,  Q).toString(buf));
	fprintf(out, "Biquad f17 = %s;\n", Biquad(sampleRate, 5230.0,  Q).toString(buf));
	fprintf(out, "Biquad f18 = %s;\n", Biquad(sampleRate, 7050.0,  Q).toString(buf));
	fprintf(out, "Biquad f19 = %s;\n", Biquad(sampleRate, 9504.0,  Q).toString(buf));
	fprintf(out, "Biquad f20 = %s;\n", Biquad(sampleRate, 12811.0, Q).toString(buf));
    fclose(out);

	return 0;
}
