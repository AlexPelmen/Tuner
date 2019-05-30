//
//	Declaration of functions from `Analize` class
//


#include "Analize.h"
#include <string.h>
#include <cmath>
#include "Smoothiner.h"

extern int gl_sample_rate;
extern int gl_asio_buffer_length;
const char * NOTES[] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };	//Names of the notes



//initialize note arrays
Analize::Analize()
{
	init_note_arrays();
	Smooth = new Smoothiner();
}



//Analize fft and gets the playing note
//
//	float fft - frequency response
//	char* out - string, where the name of playing note would be written
//
char* Analize::get_current_note( float* fft )
{
	int index = get_maximum_index(fft);
	if (index < 10) {
		return (char*)"";
	}
	int note = note_indexes_table[index];
	if ( note == -1 ) {		//means that maximum is somewhere between notes
		note = get_nearest_note(index);
	}
	Smooth->add_note( note );
	char* note_name = Smooth->get_note();
	
	return note_name;
}



//it's needed to fill array note_name_table
void Analize::init_note_arrays() {

	//normalize
	for (int i = 0; i<FFT_LEN; ++i)
		freqTable[i] = (gl_sample_rate*i) / (float)(FFT_LEN);

	//init arrays with default values
	for (int i = 0; i<FFT_LEN; ++i) {
		note_indexes_table[i] = -1;
		note_pitch_table[i] = -1;
	}
	//magic
	for (int i = 0; i<127; ++i) {
		float pitch = (440.0 / 32.0) * pow(2, (i - 9.0) / 12.0);
		if (pitch > gl_sample_rate / 2.0)
			break;
		//find the closest frequency using brute force.
		float min = 1000000000.0;
		int index = -1;
		for (int j = 0; j<FFT_LEN; ++j) {
			if (fabsf(freqTable[j] - pitch) < min) {
				min = fabsf(freqTable[j] - pitch);
				index = j;
			}
		}
		note_indexes_table[index] = i%12;
		note_pitch_table[index] = pitch;
	}
}




//gets index of the highest peak from fft
//
//	float*	fft - frequency response
//
int Analize::get_maximum_index(float * fft ){

	if (!gl_asio_buffer_length) return -1;	//err

	float cur_max = 0;
	int cur_index_of_max = 0;
	for (int i = 0; i < gl_asio_buffer_length / 2; i++) {
		if (cur_max < fft[i]) {
			cur_max = fft[i];
			cur_index_of_max = i;
		}
	}
	return cur_index_of_max;
}




//gets the nearest note to the current index form note_name_table
//
//	int index - the current index in the note_name_table_array
//	
//	returns index of the note
int Analize::get_nearest_note(int index )
{
	int up_index = index + 1;
	int down_index = index - 1;
	int note = -1;

	while ( note == -1 ) {
		if (up_index < FFT_LEN - 1) {
			if (note_indexes_table[up_index++])
				note = note_indexes_table[up_index - 1];
			else if (down_index >= 0) {
				if (note_indexes_table[down_index--])
					note = note_indexes_table[down_index + 1];
			}
		}
		else
			return -1;
	}
	return note;
}
