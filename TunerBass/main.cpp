#include <iostream>

#include "bass.h"
#include "bassasio.h"

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
Analize *Anal = new Analize();	//Analize class...
BASS_ASIO_INFO info;			//info about settings of driver

//STREAMS and appropriate buffers
HSTREAM left_channel_stream;	//left headphone
HSTREAM right_channel_stream;	//right headphone
HSTREAM gate_channel;			//for gated signal
HSTREAM buffer_for_fft;			//process fft with this stream

//Buffers
void *signal_graph_buffer;										//buffer for signal to output it on screen						
float *gate_buffer					= new float[FFT_LEN / 2];	//buffer for gated signal
float *frequency_response_buffer	= new float[FFT_LEN / 2];	//frequency response buffer	

//GLOBALS
int gl_sample_rate;
int gl_asio_buffer_length;
bool VISUALIZE = 1;		//output graphs or not


void GraphInit(){
	//resize window (it doesn't work as I expected, but it's ok )
	char Title[1024];
	GetConsoleTitle(Title, 1024);
	HWND hwnd = FindWindow(NULL, Title); 
	MoveWindow(hwnd, 0, 0, 620, 600, TRUE); 

	Graph->set_asio_buffer_length( (int)info.bufpref*0.80 );	//set buffer length for signal graph
	gl_asio_buffer_length = FFT_LEN / 2;
}

//THREADS
//Thread to process fft
//read from frequency_responce_buffer fft then returns the name of the note
void freq_res_proc() {	
	Anal->get_current_note_from_fft(frequency_response_buffer );	
}
void signal_graph_proc() {
	Graph->clear();				//paint it black
	Graph->draw_axis();			//draw coordinate plane	
	Graph->draw_sample((float*)signal_graph_buffer, 0);

	//draw text
	char* note = Anal->get_note();
	if (note && note[0] != '\0') {	//is not empty
		Graph->draw_text(0, -30, note, 4);
	}
}
void fft_graph_proc() {
	//frequency response graph
	FreqResGraph->clear();
	FreqResGraph->draw_axis();
	FreqResGraph->set_asio_buffer_length(FFT_LEN / 4);
	FreqResGraph->draw_sample(frequency_response_buffer, 0);
}


//Thread to output the graph
//It works every 
void visualization(){
	while (VISUALIZE) {
		thread signal_response_thread(signal_graph_proc);
		thread fft_response_thread(fft_graph_proc);

		//pause
		this_thread::sleep_for(chrono::milliseconds( VISUALIZATION_PAUSE ));
		
		signal_response_thread.join();
		fft_response_thread.join();
	}
	return;
}

#include <fstream>
#define RECORD_FILE "record"


class RECORDER {
public:
	std::ofstream fout;
	float* sample;
	int length;
	RECORDER(){
		fout.open(RECORD_FILE, ios::binary);
	}
	void rec_sample() {
		for (int i = 0; i < length / 4; i++) {
			float f = sample[i];
			fout.write((char*)&f, sizeof(f));
		}		
	}
	void add_sample( float* buffer, int len ) {
		sample = buffer;
		length = len;
	}
	~RECORDER(){
		fout.close();
	}

} recorder;



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

void do_proc() {
	recorder.rec_sample();
};

//CALLBACK
DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	DWORD fftlen = min( length, 1024 * 2 * 2 ); // limit the data to the amount required by the FFT (1024 samples)
	BASS_StreamPutData(gate_channel, buffer, fftlen);	//get signal
	filter_gate(fftlen);								//Gate
	
	BASS_ChannelGetData(buffer_for_fft, frequency_response_buffer, BASS_DATA_FFT8192);	//FFT
	
	BASS_StreamPutData(left_channel_stream, buffer, fftlen );	//send signal to left headphone 
	BASS_StreamPutData(right_channel_stream, buffer, fftlen );	//send signal to right headphone

	if (frequency_response_buffer) {
		thread frequency_response_proc(freq_res_proc);	//fft thread
		frequency_response_proc.detach();
	}

	recorder.add_sample((float*)buffer, length );
	thread recorder_thread(do_proc);
	recorder_thread.detach();

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

	//Set GLOBALS
	BASS_ASIO_GetInfo(&info);
	gl_sample_rate = BASS_ASIO_GetRate();
	gl_asio_buffer_length = info.bufpref;

	//initialization
	GraphInit();				//init graph's settings	
	Anal->init_note_arrays();	//init arrays with notes

	//set streams 
	left_channel_stream		= BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	right_channel_stream	= BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	gate_channel			= BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	buffer_for_fft			= BASS_StreamCreate(BASS_ASIO_GetRate(), 1, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0);
	
	//outout channels
	BASS_ASIO_ChannelEnableBASS(FALSE, 0, left_channel_stream, TRUE);
	BASS_ASIO_ChannelEnableBASS(FALSE, 1, right_channel_stream, TRUE);
	BASS_ASIO_ChannelSetFormat(FALSE, 0, BASS_ASIO_FORMAT_FLOAT);
	BASS_ASIO_ChannelSetFormat(FALSE, 1, BASS_ASIO_FORMAT_FLOAT);

	//input channel
	BASS_ASIO_ChannelEnable(TRUE, 0, inputProc, 0);
	BASS_ASIO_ChannelSetFormat(TRUE, 0, BASS_ASIO_FORMAT_FLOAT);

	BASS_ASIO_Start(0, 0);	//start driver
	thread visualization_thread(visualization);	//start visualization in new thread
	 
	system("pause");
	VISUALIZE = FALSE;		//kill the visualization thread
	visualization_thread.join();	
	BASS_ASIO_Stop();		
	BASS_ASIO_Free();
	return;
}


