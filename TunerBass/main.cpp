#include <iostream>
#include "../Bass/c/bass.h"
#include "../Bassasio/c/bassasio.h"
#include "Graph.h"
#include "Buffer.h"


using namespace std;

void *c_buffer;				//common buffer
int c_buffer_length;		//common buffer length
HSAMPLE c_sample;			//common sample
BOOL err = FALSE;			//error flag
GraphConsole *Graph = new GraphConsole;	
BOOL VISUALIZE = TRUE;		//visualize or not
BASS_ASIO_INFO info;		//info about settings of driver

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



void visualization(){
	while (VISUALIZE) {
		Graph->clear();				//paint it black
		Graph->draw_axis();			//draw coordinate plane	
		Graph->draw_sample((int*)c_buffer, 0 );
		this_thread::sleep_for(chrono::milliseconds(44));
	}
	return;
}


DWORD CALLBACK inputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user){
	c_buffer = buffer;
	return length;
}

DWORD CALLBACK outputProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user) {
	memcpy_s(buffer, length, c_buffer, length);
	return length;
}

void main() {		
	setlocale(LC_ALL, "Rus");
	err = BASS_ASIO_Init(0, 0);
	
	BASS_ASIO_GetInfo(&info);
	Graph->set_asio_buffer_length( info.bufpref );

	//catch error
	if (!err) {
		cout << "ERROR: " << BASS_ASIO_ErrorGetCode() << endl;
		return;
	}

	//set callbacks
	BASS_ASIO_ChannelEnable( TRUE, 0, inputProc, 0 );
	BASS_ASIO_ChannelEnable( FALSE, 0, outputProc, 0 );
	BASS_ASIO_ChannelEnable( FALSE, 1, outputProc, 0);
	
	BASS_ASIO_Start(0, 0);	//start driver
	thread visualizationTread(visualization);	//visualization in new thread

	system("pause");
	VISUALIZE = FALSE;		//kill the visualization thread
	visualizationTread.join();
	BASS_ASIO_Stop();		//stop asio and free buffers
	BASS_ASIO_Free();
	return;
}