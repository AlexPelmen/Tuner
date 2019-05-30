#include "Smoothiner.h"

extern const char * NOTES[];


//fill deque and notes_freq
Smoothiner::Smoothiner()
{
	for (int i = 0; i < SMOOTHINER_LENGTH; i++) {
		note_history.push_back(12);
	}
	notes_freq[12] = SMOOTHINER_LENGTH;
}



//add new note to deque and remove old one
//
//	int note_index - the index of the playing note
//
void Smoothiner::add_note_to_deque(int note_index)
{	
	notes_freq[note_history.front()]--;
	note_history.pop_front();

	note_history.push_back(note_index);
	notes_freq[note_index]++;
}




//add note to smoothiner
//
//	int note_index - the index of the playing note
//
void Smoothiner::add_note(int note_index)
{
	//change note_index = -1 to 12
	if (note_index == -1) {
		note_index = 12;	//means muted note (no sound)
	}
	add_note_to_deque(note_index);
	if (note_index != current_note) {	//wrong note (we must check history)
		
		//just find the most frequent note
		int max_index = 0;
		int max_number = notes_freq[0];
		for (int i = 1; i < 13; i++) 
		{
			if (max_number < notes_freq[i]) {
				max_number = notes_freq[i];
				max_index = i;
			}
		}
		current_note = max_index;
	}
}



//get the name of the playing note
//
//	returns char* - name of the note
//
char* Smoothiner::get_note()
{
	if (current_note == 12) {
		return (char*)"";
	}
	return (char*)NOTES[current_note];
}
