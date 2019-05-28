#include "Analize.h"
#include <cmath>

extern int gl_sample_rate;
extern int gl_asio_buffer_length;

//this method starts in the init function
//it's needed to fill array noreNameTable
void Analize::init_note_arrays() {

	//normalize
	for (int i = 0; i<FFT_LEN; ++i)
		freqTable[i] = (gl_sample_rate*i) / (float)(FFT_LEN);

	//init arrays with default values
	for (int i = 0; i<FFT_LEN; ++i) {
		noteNameTable[i] = 0;
		notePitchTable[i] = -1;
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
		noteNameTable[index] = (char*)NOTES[i % 12];
		notePitchTable[index] = pitch;
	}
}

int Analize::get_maximum_index(float * sample){

	if (!gl_asio_buffer_length) return -1;	//err

	float cur_max = 0;
	int cur_index_of_max = 0;
	for (int i = 0; i < gl_asio_buffer_length / 2; i++) {
		if (cur_max < sample[i]) {
			cur_max = sample[i];
			cur_index_of_max = i;
		}
	}
	return cur_index_of_max;
}


char * Analize::get_nearest_note(int index)
{
	int up_index = index + 1;
	int down_index = index - 1;
	char* note = 0x0000;
	current_note = new char[2];

	while (!note) {
		if (up_index < FFT_LEN - 1) {
			if (noteNameTable[up_index++])
				note = noteNameTable[up_index - 1];
			else if (down_index >= 0) {
				if (noteNameTable[down_index--])
					note = noteNameTable[down_index + 1];
			}
		}
		else
			return (char*)"#";
	}
	current_note = note;
}
