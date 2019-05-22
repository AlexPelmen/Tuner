#include <iostream>

#include "../Bass/c/bass.h"
#include "../Bassasio/c/bassasio.h"
#include "../Bassmix/c/bassmix.h"
#include "../Bassmix/c/bassmix.h"

#include "Graph.h"
#include "Buffer.h"
#include "Analize.h"
#include "fft.h"

#define FREQ_RES_PAYBACKS_NUM 8192


using namespace std;

void *c_buffer;				//common buffer
void *long_buffer;			
float *c_freq_res;			//frequency response
GraphConsole *Graph = new GraphConsole(	0, 0, 600, 300,	20,	20,	0 );	
GraphConsole *FreqResGraph = new GraphConsole(0, 300, 600, 300, 20, 20, 0);
BOOL VISUALIZE = TRUE;		//visualize or not
BASS_ASIO_INFO info;		//info about settings of driver

HSTREAM left_channel_stream;	//left headphone
HSTREAM right_channel_stream;	//right headphone
HSTREAM buffer_for_fft;
HSTREAM gate_channel;

Analize *Anal = new Analize();

int c_length = 0;
int samples_in_buffer = 0;

FFTW *fft = new FFTW();

float *gate_buffer;

//output info
void outputAsioInfo(BASS_ASIO_INFO *info) {
	cout << "Name: " << info->name << endl;
	cout << "Version: " << info->version << endl;
	cout << "Inputs: " << info->inputs << endl;
	cout << "outputs: " << info->outputs << endl;
	cout << "bufmin: " << info->bufmin << endl;
	cout << "bufmax: " << info->bufmax << endl;
	cout << "bufpref: " << info->bufpref << endl;
	cout << "bufgran: " << info->bufgran << endl;
	cout << "initflags: " << info->initflags << endl;
	cout << "Rate: " << BASS_ASIO_GetRate() << endl;
}

//get frequency
int get_freq(int payback_index, int payback_num, int sample_rate) {
	return (float)payback_index / payback_num * sample_rate*2;
}


//Stoled from github... 
float freqTable[FREQ_RES_PAYBACKS_NUM];
char * noteNameTable[FREQ_RES_PAYBACKS_NUM];
float notePitchTable[FREQ_RES_PAYBACKS_NUM];
const char * NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

void setNotes() {
	for (int i = 0; i<FREQ_RES_PAYBACKS_NUM; ++i) {
		freqTable[i] = (BASS_ASIO_GetRate() *i) / (float)(FREQ_RES_PAYBACKS_NUM);
	}
	for (int i = 0; i<FREQ_RES_PAYBACKS_NUM; ++i) {
		noteNameTable[i] = NULL;
		notePitchTable[i] = -1;
	}
	for (int i = 0; i<127; ++i) {
		float pitch = (440.0 / 32.0) * pow(2, (i - 9.0) / 12.0);
		if (pitch > BASS_ASIO_GetRate() / 2.0)
			break;
		//find the closest frequency using brute force.
		float min = 1000000000.0;
		int index = -1;
		for (int j = 0; j<FREQ_RES_PAYBACKS_NUM; ++j) {
			if (fabsf(freqTable[j] - pitch) < min) {
				min = fabsf(freqTable[j] - pitch);
				index = j;
			}
		}
		noteNameTable[index] = (char*)NOTES[i % 12];
		notePitchTable[index] = pitch;
		//printf( "%f %d %s\n", pitch, index, noteNameTable[index] );
	}
}

void GraphInit(){
	//resize window
	HWND hwnd;
	char Title[1024];
	GetConsoleTitle(Title, 1024);
	hwnd = FindWindow(NULL, Title); 
	MoveWindow(hwnd, 0, 0, 620, 600, TRUE); 

	Graph->set_asio_buffer_length( (int)info.bufpref*0.80 );
	FreqResGraph->set_asio_buffer_length( 512 );
	Anal->set_buffer_length((int)info.bufpref);

	setNotes();
}


//Thread to output the graph
void visualization(){
	while (VISUALIZE) {
		//signal's graph 
		Graph->clear();				//paint it black
		Graph->draw_axis();			//draw coordinate plane	
		Graph->draw_sample((float*)c_buffer, 0 );
							
		//frequency response graph
		FreqResGraph->clear();
		FreqResGraph->draw_axis();
		FreqResGraph->set_asio_buffer_length(FREQ_RES_PAYBACKS_NUM /4);
		FreqResGraph->draw_sample(c_freq_res, 0 );

		if (c_freq_res) {
			system("cls");
			int index = Anal->get_maximum_index(c_freq_res);
			char* note = noteNameTable[index];
			if (!note) {
				//search note in the array
				int up_index = index + 1;
				int down_index = index - 1;
				while (!note) {
					if ( up_index >= 0 && up_index < FREQ_RES_PAYBACKS_NUM - 1 )
						if( noteNameTable[up_index++] )
							note = noteNameTable[up_index];
					else if (down_index >= 0 && down_index < FREQ_RES_PAYBACKS_NUM - 1)
						if (noteNameTable[down_index--])
							note = noteNameTable[down_index];
				}
			}
			cout << note << endl;
		}
		this_thread::sleep_for(chrono::milliseconds(44));
	}
	return;
}


//gate to remove noise
float trashold = 0.01;
void filter_gate(int length) {
	BASS_ChannelGetData(gate_channel, gate_buffer, length );
	if (gate_buffer) {
		for (int i = 0; i < length; i++) {
			float val = gate_buffer[i];
			if (abs(val) < trashold)
				gate_buffer[i] = 0;
		}
	}
	BASS_StreamPutData(buffer_for_fft, gate_buffer, length);
}

//CALLBACK
DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	DWORD fftlen = min( length, 1024 * 2 * 2 ); // limit the data to the amount required by the FFT (1024 samples)
	
	if( ! c_freq_res )
		c_freq_res = new float[FREQ_RES_PAYBACKS_NUM/2];
	if (!gate_buffer)
		gate_buffer = new float[FREQ_RES_PAYBACKS_NUM/2];

	BASS_StreamPutData(gate_channel, buffer, fftlen);
	filter_gate(fftlen);
	BASS_ChannelGetData(buffer_for_fft, c_freq_res, BASS_DATA_FFT8192);

	BASS_StreamPutData(left_channel_stream, buffer, fftlen );
	BASS_StreamPutData(right_channel_stream, buffer, fftlen );

	c_buffer = buffer;
	c_length = length;
	samples_in_buffer++;
	return length;	
}



void main() {		
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