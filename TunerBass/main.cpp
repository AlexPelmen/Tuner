#include <iostream>

#include "../Bass/c/bass.h"
#include "../Bassasio/c/bassasio.h"
#include "../Bassmix/c/bassmix.h"
#include "../Bassmix/c/bassmix.h"

#include "Graph.h"
#include "Buffer.h"
#include "Analize.h"
#include "fft.h"


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

Analize *Anal = new Analize();

int c_length = 0;
int samples_in_buffer = 0;

FFTW *fft = new FFTW();



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
}


//Thread to output the graph
void visualization(){
	while (VISUALIZE) {
		//signal's graph 
		Graph->clear();				//paint it black
		Graph->draw_axis();			//draw coordinate plane	
		Graph->draw_sample((float*)c_buffer, 0 );

		//TEMPORARY!!!
		if (c_length && buffer_for_fft) {	//fft (maybe it's better to create new function for this stuff)				

			//fill ftt->in array with double 		
			float* data = new float[c_length];	//need this to retrive data from stream	
			int samples_num = samples_in_buffer;	//remember this number cause of multithreading (samples_in_buffer would be changed latter in other thread)
			cout << samples_num << endl;
			int buf_len = c_length * samples_num;
			samples_in_buffer = 0;

			fft->set_in_len( buf_len );	//allocate memory for fft in-array
			fft->set_out_len( buf_len );	//allocate memory for fft out-array	
			
			for (int i = 0; i < samples_num; i++) {			//iter in buffer
				BASS_ChannelGetData( buffer_for_fft, data, BASS_DATA_FFT1024);
				for (int sam_i = 0; sam_i < c_length; sam_i++)		//iter in sample
					fft->push_in((double)data[sam_i]*1000);
			}

			delete data;

			//do fft
			fft->fft(buf_len);

			//fill c_freq_res with floats from coplex-array
			double* val;
			float fl = 0;			
			c_freq_res = new float[fft->len_out];
			for (int i = 0; i < fft->len_out; i++) {
				val = fft->out[i];
				fl = val[0];
				c_freq_res[i] = fl;	//just real
			}
		}

		//normalization
		for (int x = 0; x < fft->len_out; x++) {
			c_freq_res[x] = c_freq_res[x] / fft->len_in/10000000000;
			if ( abs(c_freq_res[x]) > 1) c_freq_res[x] = 1;
		}
		if (c_freq_res) {
			float mass[10];
			for (int i = 0; i < 10; i++) {
				mass[i] = c_freq_res[i];
			}
		}
					
		//frequency response graph
		FreqResGraph->clear();
		FreqResGraph->draw_axis();
		FreqResGraph->set_asio_buffer_length( fft->len_out );
		FreqResGraph->draw_sample( c_freq_res, 0 );

		delete c_freq_res;	//don't need this stuff till the next iteration

		this_thread::sleep_for(chrono::milliseconds(44));
	}
	return;
}


//CALLBACKS
DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	DWORD fftlen = min( length, 1024 * 2 * 2 ); // limit the data to the amount required by the FFT (1024 samples)
	if( ! c_freq_res )
		c_freq_res = new float[fftlen];
	BASS_StreamPutData( buffer_for_fft, buffer, fftlen);
	BASS_StreamPutData(left_channel_stream, buffer, fftlen );
	BASS_StreamPutData(right_channel_stream, buffer, fftlen );
	//BASS_ChannelGetData( left_channel_stream, c_freq_res, BASS_DATA_FFT1024); // get back the FFT data

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