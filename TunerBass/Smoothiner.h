#pragma once
#include "ProcConfig.h"
#include <deque>

class Smoothiner{
public:
	Smoothiner();
private:	
	int current_note;		//CURRENT NOTE 
	int notes_freq[13];		//how often every note has been met  (12 notes + 'no sound' )

	std::deque<int> note_history;	//deque with some notes 

	void add_note_to_deque(int note_index);

public:
	void add_note(int note_index);
	char* get_note();	
};

