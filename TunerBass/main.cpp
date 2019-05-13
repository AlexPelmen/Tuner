#include <iostream>

#include "../Bass/c/bass.h"
#include "../Bassasio/c/bassasio.h"
#include "../Bassmix/c/bassmix.h"

#include "Graph.h"
#include "Buffer.h"


using namespace std;

void *c_buffer;				//common buffer
HSAMPLE c_sample;			//common sample
BOOL err = FALSE;			//error flag
GraphConsole *Graph = new GraphConsole;	
BOOL VISUALIZE = TRUE;		//visualize or not
BASS_ASIO_INFO info;		//info about settings of driver

HSTREAM common_stream;		//!!!


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


//Thread to output the graph
void visualization(){
	while (VISUALIZE) {
		Graph->clear();				//paint it black
		Graph->draw_axis();			//draw coordinate plane	
		Graph->draw_sample((int*)c_buffer, 0 );
		this_thread::sleep_for(chrono::milliseconds(44));
	}
	return;
}


//CALLBACKS
DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	BASS_StreamPutData( common_stream, buffer, length);
	c_buffer = buffer;
	return length;
}
DWORD CALLBACK outputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user) {
	memcpy_s(buffer, length, c_buffer, length);
	return length;
}


void main() {		
	setlocale(LC_ALL, "Rus");
	
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
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
	Graph->set_asio_buffer_length( info.bufpref );	
	cout << BASS_ErrorGetCode() << endl;

	//set callbacks	
	/*BASS_ASIO_ChannelEnable( TRUE, 0, inputProc, 0 );
	BASS_ASIO_ChannelEnable( FALSE, 0, outputProc, 0);
	BASS_ASIO_ChannelEnable( FALSE, 1, outputProc, 0);*/

	cout << "Before all: " << BASS_ErrorGetCode() << endl;
	//HSTREAM stream_to_file = BASS_StreamCreateFile( 0, "C:/Users/Pelmen/Desktop/Rosemary.mp3", 0, 0, BASS_STREAM_DECODE);
	common_stream = BASS_StreamCreate(BASS_ASIO_GetRate(), 2, BASS_STREAM_DECODE, STREAMPROC_PUSH, 0 );
	cout << "Stream created: " << BASS_ErrorGetCode() << endl;
	
	//outout channels
	BASS_ASIO_ChannelEnableBASS(FALSE, 0, common_stream, TRUE);
	BASS_ASIO_ChannelSetFormat(FALSE, 0, BASS_ASIO_FORMAT_FLOAT);
	BASS_ASIO_ChannelSetFormat(FALSE, 1, BASS_ASIO_FORMAT_FLOAT);
	cout << "Output channel: " << BASS_ASIO_ErrorGetCode() << endl;

	//input channel
	BASS_ASIO_ChannelEnable(TRUE, 0, inputProc, 0);
	BASS_ASIO_ChannelSetFormat(TRUE, 0, BASS_ASIO_FORMAT_FLOAT);


	/*BASS_ASIO_ChannelEnableBASS(FALSE, 0, mixer, TRUE);
	cout << BASS_ErrorGetCode() << endl;
	BASS_ChannelPlay(mixer, 0);*/

	BASS_ASIO_Start(0, 0);	//start driver
	//thread visualizationTread(visualization);	//visualization in new thread

	system("pause");
	VISUALIZE = FALSE;		//kill the visualization thread
	//visualizationTread.join();
	BASS_ASIO_Stop();		//stop asio and free buffers
	BASS_ASIO_Free();
	return;
}