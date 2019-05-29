#pragma once
#include "ProcConfig.h"
#include <deque>

class Smoothiner{
private:	
	int current_note;		//CURRENT NOTE 
	int notes_freq[12];		//how often every note has been met 

	std::deque<int> note_history;

	void add_note_to_deque(int note_index);

public:
	void add_note(int note_index);
	char* get_note();	
};

