#include "Smoothiner.h"

extern const char * NOTES[];

void Smoothiner::add_note_to_deque(int note_index)
{
	if (note_history.size() >= SMOOTHINER_LENGTH) {
		notes_freq[note_history.back()]--;
		note_history.pop_front();
	}
	note_history.push_back(note_index);
	notes_freq[note_index]++;
}


void Smoothiner::add_note(int note_index)
{
	add_note_to_deque(note_index);
	if (note_index != current_note) {	//wrong note (we must check history)
		
		//just find the most frequent note
		int max_index = 0;
		int max_number = notes_freq[0];
		for (int i = 1; i < SMOOTHINER_LENGTH; i++) 
		{
			if (max_number > notes_freq[i]) {
				max_number = notes_freq[i];
				max_index = i;
			}
		}
		current_note = max_index;
	}
}


char* Smoothiner::get_note()
{
	return (char*)NOTES[current_note];
}
