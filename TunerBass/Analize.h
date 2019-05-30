#pragma once
#include "ProcConfig.h" 
#include "Smoothiner.h"

class Analize {
public:
	Analize();
	~Analize() {};

	char current_note[3];					//CURRENT NOTE!!!
		 
	float freqTable[FFT_LEN];				//array of frequencies as they appropriate fft-spectre
	int note_indexes_table[FFT_LEN];		//the same as a previews, but array containts names fo the notes
	float note_pitch_table[FFT_LEN];		//array of pitches
	
	char* get_current_note( float* sample );	//set's current note that is playing now	
	void init_note_arrays();
	int get_maximum_index(float* sample);
	int get_nearest_note(int index );	

	Smoothiner* Smooth;
};