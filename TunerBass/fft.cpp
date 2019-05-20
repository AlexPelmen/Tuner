#pragma once
#include "fft.h"

//constructor. Set iters to zero
FFTW::FFTW() {
	iter_in = 0;
	iter_out = 0;
	len_in = 0;
	len_out = 0;
};

//destructor. Deallocates arrays and plan
FFTW::~FFTW(){
	fftw_destroy_plan(p);
	fftw_free(in); 
	fftw_free(out);
}

//set new buffer length for input array
void FFTW::set_in_len( int N) {
	lim_in = N;
	fftw_free(in);
	iter_in = 0;
	len_in = 0;
	in = (double*)fftw_malloc(sizeof(double) * (N+1));
}
//set new buffer length for output array
void FFTW::set_out_len(int N) {
	lim_out = N;
	fftw_free(out);
	iter_out = 0;
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N+1));
}

//do fft (arrays must be arranged yet)
void FFTW::fft( int buf_len ) {
	if (!buf_len) return;
	fftw_destroy_plan(p);
	p = fftw_plan_dft_r2c_1d(buf_len - 1, in, out, FFTW_ESTIMATE);
	last_buf_len = buf_len;	
	fftw_execute(p);	

	float inp[10];
	float outp[10];
	for (int i = 0; i < 10; i++) {
		inp[i] = in[i];
		outp[i] = out[i][0];
	}
	len_out = len_in / 2 + 1;
}

//adds element to in-array
void FFTW::push_in( double a ) {
	in[iter_in++] = a;
	len_in++;
}



