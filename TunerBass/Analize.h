#pragma once
#include "ProcConfig.h" 

class Analize {
public:
	Analize();
	~Analize() {};

	char current_note[3];					//CURRENT NOTE!!!
		 
	float freqTable[FFT_LEN];				//array of frequencies as they appropriate fft-spectre
	char* note_name_table[FFT_LEN];			//the same as a previews, but array containts names fo the notes
	float note_pitch_table[FFT_LEN];		//array of pitches
	
	void get_current_note(float* sample, char* note );	//set's current note that is playing now	
	void init_note_arrays();
	int get_maximum_index(float* sample);
	char* get_nearest_note(int index, char* out);	
};