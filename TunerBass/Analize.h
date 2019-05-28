#pragma once
#include "ProcConfig.h" 

class Analize {
public:
	Analize() {};
	Analize(int buffer_length);
	~Analize() {};

	/*int asio_buffer_length;
	int sample_rate;*/
	char* current_note;				//current note playing
	float freqTable[FFT_LEN];		//array of frequencies as they appropriate fft-spectre
	char * noteNameTable[FFT_LEN];	//the same as a previews, but array containts names fo the notes
	float notePitchTable[FFT_LEN];	//array of pitches
		
	void init_note_arrays();
	void set_buffer_length(int buffer_length); 
	int get_maximum_index(float* sample);
	char* get_nearest_note(int index);
};