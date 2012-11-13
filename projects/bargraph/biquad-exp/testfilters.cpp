
#include <stdio.h>
#include <math.h>

#include "biquad.h"


void plot1t(Filter* const filter, int npoints) {
	char filename[1024];

	sprintf(filename, "bq-%d-1t-f.txt", (int) filter->freq());

    FILE* plot = fopen(filename, "w");

    sprintf(filename, "# Biquad bandpass Fc=%d Hz 1(t) response, Q=%1.2f, scale=%d\n", 
    	(int) filter->freq(), filter->Q(), filter->scale());
    fprintf(plot, filename);

	filter->ffilter(0.0);
	for(int i = 0; i < npoints; i++) {
		fprintf(plot, "%f\t%f\n", 1000.0*i/filter->sampleRate(), filter->ffilter(1.0));
	}
	fclose(plot);

	sprintf(filename, "bq-%d-1t-i.txt", (int) filter->freq());
    plot = fopen(filename, "w");

    sprintf(filename, "# %d Hz 1(t) response, Q=%1.2f, scale=%d\n", 
    	(int) filter->freq(), filter->Q(), filter->scale());
    fprintf(plot, filename);

	filter->ifilter(0.0);
	for(int i = 0; i < npoints; i++) {
		fprintf(plot, "%f\t%f\n", 1000.0*i/filter->sampleRate(), filter->ifilter(filter->scale())/(1.0*filter->scale()));
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

		fprintf(plot, "%f\t%f  # \n", 1.0*filter->sampleRate() * i / (len - 1) / 2, y, 0, 0);
	}
	fclose(plot);
}

int main(int argc, char **argv) {
	int sampleRate = 28000;
	plot1t(&Biquad<65536>(sampleRate, 44.0, 2.0), 4096);  
	plot1t(&Biquad<65536>(sampleRate, 60.0, 1.5), 4096);
	plot1t(&Biquad<32768>(sampleRate, 80.0, 1.5), 4096);
	plot1t(&Biquad<32768>(sampleRate, 107.0, 1.1), 4096);
	plot1t(&Biquad<32768>(sampleRate, 145.0, 1.0), 4096);
	plot1t(&Biquad<32768>(sampleRate, 195.0, 0.8), 2048);
	plot1t(&Biquad<32768>(sampleRate, 264.0, 0.75), 2048);
	plot1t(&Biquad<32768>(sampleRate, 356.0, 0.74), 1024);
	plot1t(&Biquad<32768>(sampleRate, 480.0, 1.0), 1024);
	plot1t(&Biquad<32768>(sampleRate, 647.0, 1.0), 512);
	plot1t(&Biquad<32768>(sampleRate, 872.0, 1.0), 512);
	plot1t(&Biquad<32768>(sampleRate, 1175.0, 1.0), 512);
	plot1t(&Biquad<32768>(sampleRate, 1584.0, 1.0), 256);
	plot1t(&Biquad<32768>(sampleRate, 2135.0, 1.0), 256);
	plot1t(&Biquad<32768>(sampleRate, 2878.0, 1.0), 256);
	plot1t(&Biquad<32768>(sampleRate, 3880.0, 1.0), 128);
	plot1t(&Biquad<32768>(sampleRate, 5230.0, 1.0), 128);
	plot1t(&Biquad<32768>(sampleRate, 7050.0, 1.0), 128);
	plot1t(&Biquad<32768>(sampleRate, 9504.0, 1.0), 128);
	plot1t(&Biquad<32768>(sampleRate, 12811.0, 0.6), 128);


	return 0;
}