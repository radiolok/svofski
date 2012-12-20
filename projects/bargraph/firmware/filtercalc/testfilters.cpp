
#include <stdio.h>
#include <math.h>

#include "../include/fixp.h"
#include "../include/biquad2.h"


void plot1t(Biquad* const filter, int npoints) {
	char filename[1024];

	sprintf(filename, "bq-%d-1t-f.txt", (int) filter->freq());

    FILE* plot = fopen(filename, "w");

    sprintf(filename, "# Biquad bandpass Fc=%d Hz 1(t) response, Q=%1.2f, scale=%d\n", 
    	(int) filter->freq(), filter->Q(), filter->scale());
    fprintf(plot, filename);

	filter->ffilter(0.0);
	for(int i = 0; i < npoints; i++) {
		fprintf(plot, "%f\t%f\n", 1000.0*i/SAMPLERATE, filter->ffilter(1.0));
		//fprintf(plot, "%f\t%f\n", 1000.0*i/SAMPLERATE, 0*filter->ffilter(sin(1000.0*i/SAMPLERATE)));
	}
	fclose(plot);

	sprintf(filename, "bq-%d-1t-i.txt", (int) filter->freq());
    plot = fopen(filename, "w");

    sprintf(filename, "# %d Hz 1(t) response, Q=%1.2f, scale=%d\n", 
    	(int) filter->freq(), filter->Q(), filter->scale());
    fprintf(plot, filename);

	filter->ifilter(0.0);
	for(int i = 0; i < npoints; i++) {
        float t = 1000.0 * i/SAMPLERATE;
        int filtered = filter->ifilter(FIXP_ONE-FIXP_ONE/6);
        //float mag = 20*log10(fabs(filtered)/filter->scale());
		//fprintf(plot, "%f\t%f\n", 1000.0*i/SAMPLERATE, mag);
		fprintf(plot, "%f\t%f\n", 1000.0*i/SAMPLERATE, 1.0*filtered/FIXP_ONE);
	}
	fclose(plot);

	// now the frequency response

	sprintf(filename, "fr-%d.txt", (int)filter->freq());
    plot = fopen(filename, "w");

    int len = 8192;
	for (int i = 0; i < len; i++) {
		//float w = exp(log(1 / 0.001) * i / (512 - 1)) * 0.001 * M_PI;
		float w = 1.0*i / (len - 1) * M_PI;
		float phi = pow(sin(w/2), 2);

		float a0 = filter->A0();
		float a1 = filter->A1();
		float a2 = filter->A2();
		float b1 = filter->B1();
		float b2 = filter->B2();

		float y = log(pow(a0+a1+a2, 2) - 4*(a0*a1 + 4*a0*a2 + a1*a2)*phi + 16*a0*a2*phi*phi) 
  			- log(pow(1+b1+b2, 2) - 4*(b1 + 4*b2 + b1*b2)*phi + 16*b2*phi*phi);
		y = y * 10 / log(10);

		fprintf(plot, "%f\t%f  # \n", 1.0*SAMPLERATE * i / (len - 1) / 1, y, 0, 0);
	}
	fclose(plot);
}

#include "./instfilters.h"

int main(int argc, char **argv) {
    plot1t(&f1, 4096);
    plot1t(&f2, 4096);
    plot1t(&f3, 4096);
    plot1t(&f4, 4096);
    plot1t(&f5, 4096);
    plot1t(&f6, 2048);
    plot1t(&f7, 2048);
    plot1t(&f8, 2048);
    plot1t(&f9, 2048);
    plot1t(&f10, 1024);
    plot1t(&f11, 1024);
    plot1t(&f12, 1024);
    plot1t(&f13, 1024);
    plot1t(&f14, 1024);
    plot1t(&f15, 1024);
    plot1t(&f16, 512);
    plot1t(&f17, 512);
    plot1t(&f18, 128);
    plot1t(&f19, 128);
    plot1t(&f20, 128);
    /*
	plot1t(&Biquad(sampleRate, 44.0, 2.0), 4096);  
	plot1t(&Biquad(sampleRate, 60.0, 1.5), 4096);
	plot1t(&Biquad(sampleRate, 80.0, 1.5), 4096);
	plot1t(&Biquad(sampleRate, 107.0, 1.1), 4096);
	plot1t(&Biquad(sampleRate, 145.0, 1.0), 4096);
	plot1t(&Biquad(sampleRate, 195.0, 0.8), 2048);
	plot1t(&Biquad(sampleRate, 264.0, 0.75), 2048);
	plot1t(&Biquad(sampleRate, 356.0, 0.74), 1024);
	plot1t(&Biquad(sampleRate, 480.0, 1.0), 1024);
	plot1t(&Biquad(sampleRate, 647.0, 1.0), 512);
	plot1t(&Biquad(sampleRate, 872.0, 1.0), 512);
	plot1t(&Biquad(sampleRate, 1175.0, 1.0), 512);
	plot1t(&Biquad(sampleRate, 1584.0, 1.0), 256);
	plot1t(&Biquad(sampleRate, 2135.0, 1.0), 256);
	plot1t(&Biquad(sampleRate, 2878.0, 1.0), 256);
	plot1t(&Biquad(sampleRate, 3880.0, 1.0), 128);
	plot1t(&Biquad(sampleRate, 5230.0, 1.0), 128);
	plot1t(&Biquad(sampleRate, 7050.0, 1.0), 128);
	plot1t(&Biquad(sampleRate, 9504.0, 1.0), 128);
	plot1t(&Biquad(sampleRate, 12811.0, 0.6), 128);
*/

	return 0;
}
