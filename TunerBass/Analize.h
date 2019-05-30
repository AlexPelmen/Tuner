#pragma once
#include "ProcConfig.h" 
#include "Smoothiner.h"

class Analize {
public:
	Analize();

private:
	char* current_note;						//CURRENT NOTE!!!		 
	
	int note_indexes_table[FFT_LEN];		//the same as a previews, but array containts names fo the notes		
	
	int get_maximum_index(float* sample);	//get the highest peak from fft
	int get_nearest_note(int index );		//retrive the nearest note to current index in the note_index_array
	Smoothiner* Smooth;						//Smoothiner...

public:	
	void init_note_arrays();						//fill note_index_array
	char* get_note();								//returns current note		
	char* get_current_note_from_fft( float* fft );	//set's current note that is playing now	
};