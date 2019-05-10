#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <thread>

class GraphConsole {
public:
	int paddingX = 30;
	int paddingY = 30;
	int width;
	int height;
	int centerX;
	int centerY;

	float scale = 1;			//scale the graph
	int asio_buffer_length;		//buffer length
	
	//handles
	RECT wndRect;				//client window rect
	HWND hWnd;					//handle to the window
	HDC hDC;					//handle to DC
	HPEN WhitePen;				//white pen (axes)
	HBRUSH BlackBrush;			//black brush (background)

	GraphConsole();				//constructor

	void set_asio_buffer_length(int buffer_length);
	void clear();				//paint it black
	void draw_axis();			//draw coordinate plane					
	void draw_new_point(float x, float y);		//add new point to the graph
	void draw_sample(int* sample, int offset);	//output sample to the graph
};
