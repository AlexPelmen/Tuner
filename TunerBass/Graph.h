#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <thread>

class GraphConsole {
public:
	int offset_x;	//coord x of the graph box
	int offset_y;	//coord y of the graph box
	int padding_x;	//horizontal indent from side of the window to client area
	int padding_y;	//vertical indent from side of the window to client area
	int width;		//width of the graph box
	int height;		//height of the graph box
	int centerX;	//x coord of the center
	int centerY;	//y coord of the center
	int client_width;	//width of the client box
	int client_height;	//height of the client box
	float scale = 1;			//scale the graph (default 1, but it would be recounted )
	int asio_buffer_length;		//buffer length (from asio)

	GraphConsole(
		int x,
		int y,
		int width,
		int height,
		int padding_x,
		int padding_y,
		int buffer_length
	);
	
	//handles
	RECT wndRect;				//client window rect
	HWND hWnd;					//handle to the window
	HDC hDC;					//handle to DC
	HPEN WhitePen;				//white pen (axes)
	HBRUSH BlackBrush;			//black brush (background)
	
	void set_asio_buffer_length(int buffer_length);
	void clear();				//paint it black
	void draw_axis();			//draw coordinate plane					
	void draw_new_point(float x, float y);		//add new point to the graph
	void draw_sample(float* sample, int offset);	//output sample to the graph
};
