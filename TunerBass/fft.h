#pragma once
#include <fftw3.h>

class FFTW {
public:
	double *in;
	fftw_complex *out;
	fftw_plan p;
	int last_buf_len;

	FFTW();
	~FFTW();
	void fft( int buf_len );

	//arrays
	int iter_in;
	int iter_out;
	int len_in;
	int len_out;
	int lim_in;
	int	lim_out;
	void set_in_len(int N);
	void set_out_len(int N);
	void push_in(double a);


	
};