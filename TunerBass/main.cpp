#include <iostream>

#include "../Bass/c/bass.h"
#include "../Bassasio/c/bassasio.h"

#include "Graph.h"
#include "Buffer.h"
#include "Analize.h"
#include "ProcConfig.h"

#include "Graph_config.h";

using namespace std;

GraphConsole *Graph = new GraphConsole(
	SIGNAL_GRAPH_OFFSET_X, 
	SIGNAL_GRAPH_OFFSET_Y, 
	SIGNAL_GRAPH_WIDTH, 
	SIGNAL_GRAPH_HEIGHT,
	SIGNAL_GRAPH_PADDING_X,
	SIGNAL_GRAPH_PADDING_Y,
	0	//buffer_length (it's still unknown)
);
GraphConsole *FreqResGraph = new GraphConsole(
	FFT_GRAPH_OFFSET_X,
	FFT_GRAPH_OFFSET_Y,
	FFT_GRAPH_WIDTH,
	FFT_GRAPH_HEIGHT,
	FFT_GRAPH_PADDING_X,
	FFT_GRAPH_PADDING_Y,
	FFT_LEN
);

BASS_ASIO_INFO info;		//info about settings of driver

//STREAMS and appropriate buffers

HSTREAM left_channel_stream;	//left headphone
HSTREAM right_channel_stream;	//right headphone
void *signal_graph_buffer;		//buffer for signal to output it on screen	

HSTREAM buffer_for_fft;										//process fft with this stream
float *frequency_response_buffer = new float[FFT_LEN / 2];	//frequency response buffer

HSTREAM gate_channel;							//for gated signal
float *gate_buffer = new float[FFT_LEN / 2];	//buffer for gated signal

Analize *Anal = new Analize();	//Analize class...

//Notes
float freqTable[FFT_LEN];		//array of frequencies as they appropriate fft-spectre
char * noteNameTable[FFT_LEN];	//the same as a previews, but array containts names fo the notes
float notePitchTable[FFT_LEN];	//array of pitches
const char * NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };	//Names of the notes


//this method starts in the init function
//it's needed to fill array noreNameTable
void set_notes() {

	//normalize
	for (int i = 0; i<FFT_LEN; ++i)		
		freqTable[i] = (BASS_ASIO_GetRate() *i) / (float)(FFT_LEN);
	
	//init arrays with default values
	for (int i = 0; i<FFT_LEN; ++i) {
		noteNameTable[i] = NULL;
		notePitchTable[i] = -1;
	}

	//magic
	for (int i = 0; i<127; ++i) {
		float pitch = (440.0 / 32.0) * pow(2, (i - 9.0) / 12.0);
		if (pitch > BASS_ASIO_GetRate() / 2.0)
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

/*	Get the note from 'noteNameTable' array
*
*	Not all values would be set up in this array, so
*	if index is appropriate to NULL value, we use this funtion
*	to find the nearest note to this index
*/

//global var for the curent note
char* curent_note;

char* get_nearest_note(int index) {
	
	int up_index = index + 1;
	int down_index = index - 1;
	char* note = 0x0000;
	curent_note = new char[2];

	while (!note) {
		if (up_index < FFT_LEN - 1) {
			if (noteNameTable[up_index++])
				note = noteNameTable[up_index-1];
			else if (down_index >= 0) {
				if (noteNameTable[down_index--])
					note = noteNameTable[down_index+1];
			}
		}
		else
			return (char*)"#";
	}
	curent_note = note;
}


void GraphInit(){
	//resize window (it doesn't work as I expected, but it's ok )
	HWND hwnd;
	char Title[1024];
	GetConsoleTitle(Title, 1024);
	hwnd = FindWindow(NULL, Title); 
	MoveWindow(hwnd, 0, 0, 620, 600, TRUE); 

	Graph->set_asio_buffer_length( (int)info.bufpref*0.80 );	//set buffer length for signal graph
	Anal->set_buffer_length(FFT_LEN/2);					//the same for analizing class
}


//Thread to process fft
//read from frequency_responce_buffer fft then returns the name of the note
void freq_res_proc() {
	if (frequency_response_buffer) {
		int index = Anal->get_maximum_index(frequency_response_buffer);
		char* curent_note = noteNameTable[index];
		if (!curent_note)				//means that maximum is somewhere between notes
			get_nearest_note(index);

	}
	if (curent_note) {
		system("cls");
		printf("%s", curent_note);
	}

}

//Thread to output the graph
//It works every 
void visualization(){
	while (VISUALIZE) {
		//signal's graph 
		Graph->clear();				//paint it black
		Graph->draw_axis();			//draw coordinate plane	
		Graph->draw_sample((float*)signal_graph_buffer, 0 );
							
		//frequency response graph
		FreqResGraph->clear();
		FreqResGraph->draw_axis();
		FreqResGraph->set_asio_buffer_length(FFT_LEN /4);
		FreqResGraph->draw_sample(frequency_response_buffer, 0 );

		thread frequency_response_proc(freq_res_proc);	//fft thread
		frequency_response_proc.detach();

		//pause
		this_thread::sleep_for(chrono::milliseconds( VISUALIZATION_PAUSE ));
	}
	return;
}


/*	Gate to remove noise
*
*	signal from 'gate_channel' would be processed
*	gated signal is been put to 'buffer_for_fft'
*/
void filter_gate(int length) {
	BASS_ChannelGetData(gate_channel, gate_buffer, length );
	if (gate_buffer) {
		for (int i = 0; i < length; i++) {
			float val = gate_buffer[i];
			if (abs(val) < SIGNAL_GATE_TRASHOLD )
				gate_buffer[i] = 0;
		}
	}
	BASS_StreamPutData(buffer_for_fft, gate_buffer, length);
}


//CALLBACK
DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	DWORD fftlen = min( length, 1024 * 2 * 2 ); // limit the data to the amount required by the FFT (1024 samples)
	BASS_StreamPutData(gate_channel, buffer, fftlen);	//get signal
	filter_gate(fftlen);								//Gate
	BASS_ChannelGetData(buffer_for_fft, frequency_response_buffer, BASS_DATA_FFT8192);	//FFT

	BASS_StreamPutData(left_channel_stream, buffer, fftlen );	//send signal to left headphone 
	BASS_StreamPutData(right_channel_stream, buffer, fftlen );	//send signal to right headphone

	signal_graph_buffer = buffer;	//don't forget about buffer
	return length;	
}



void main() {		

	//Init
	setlocale(LC_ALL, "Rus");	
	if ( ! BASS_Init(-1, 44100, 0, 0, NULL)) {
		cout << "BASS ERROR: " << BASS_ErrorGetCode() << endl;
		system("pause");
		return;
	}
	if ( ! BASS_ASIO_Init(0, BASS_ASIO_THREAD ) ) {
		cout << "BASSASIO ERROR: " << BASS_ASIO_ErrorGetCode() << endl;
		system("pause");
		return;
	}	

	BASS_ASIO_GetInfo(&info);
	GraphInit();	//init graph's settings	
	set_notes();	//init arrays with notes

	//set streams 
	left_channel_stream = BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0 );
	right_channel_stream = BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0 );
	gate_channel = BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	buffer_for_fft = BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	
	//outout channels
	BASS_ASIO_ChannelEnableBASS(FALSE, 0, left_channel_stream, TRUE);
	BASS_ASIO_ChannelEnableBASS(FALSE, 1, right_channel_stream, TRUE);
	BASS_ASIO_ChannelSetFormat(FALSE, 0, BASS_ASIO_FORMAT_FLOAT);
	BASS_ASIO_ChannelSetFormat(FALSE, 1, BASS_ASIO_FORMAT_FLOAT);

	//input channel
	BASS_ASIO_ChannelEnable(TRUE, 0, inputProc, 0);
	BASS_ASIO_ChannelSetFormat(TRUE, 0, BASS_ASIO_FORMAT_FLOAT);

	BASS_ASIO_Start(0, 0);	//start driver
	thread visualizationTread(visualization);	//visualization in new thread
	 
	system("pause");
	VISUALIZE = FALSE;		//kill the visualization thread
	visualizationTread.join();
	BASS_ASIO_Stop();		//stop asio and free buffers
	BASS_ASIO_Free();
	return;
}