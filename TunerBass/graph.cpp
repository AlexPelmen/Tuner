#pragma once
#include "Graph.h"
#include <iostream>

using namespace std;

GraphConsole::GraphConsole( int x, int y, int width, int height, int padding_x, int padding_y, int buffer_length ) {
	
	GraphConsole::offset_x = x;
	GraphConsole::offset_y = y;
	GraphConsole::padding_x = padding_x;
	GraphConsole::padding_y = padding_y;
	GraphConsole::width = width;
	GraphConsole::height = height;
	GraphConsole::asio_buffer_length = buffer_length;

	client_width = width - padding_x * 2;
	client_height = height - padding_y * 2;
	centerX = offset_x + padding_x;
	centerY = offset_y + padding_y + client_height / 2;
	
	//Pen and brush
	WhitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	BlackBrush = CreateSolidBrush(RGB(0, 0, 0));

	//handles
	hWnd = GetConsoleWindow();
	hDC = GetDC( hWnd );

	scale = asio_buffer_length / client_width;
};

void GraphConsole::set_asio_buffer_length(int buffer_length) {
	asio_buffer_length = buffer_length;
	scale = (float) client_width / asio_buffer_length;
}


void GraphConsole::draw_axis() {
	SelectObject(hDC, WhitePen);
	//Horizontal
	MoveToEx(hDC, centerX, centerY, NULL);	//left point
	LineTo(hDC, offset_x + padding_x + client_width - 1, centerY );	//right point
	//Vertical
	MoveToEx(hDC, centerX, centerY, NULL);
	LineTo(hDC, centerX, centerY - client_height / 2);
	MoveToEx(hDC, centerX, centerY, NULL);
	LineTo(hDC, centerX, centerY + client_height / 2);
};

void GraphConsole::clear() {
	SelectObject(hDC, BlackBrush);
	Rectangle(
		hDC,	//handle to DC
		offset_x + padding_x, //left side of the box
		offset_y + padding_y, //top side of the box
		offset_x + width - padding_x,		//right side of the box 
		offset_y + height - padding_y + 1	//bottom side of the box
	);
};

void GraphConsole::draw_new_point(float x, float y) {
	SelectObject(hDC, WhitePen);
	float relVal = y / 2;	//it normalized yet
	int Y = round( client_height * relVal ); //get val

	//to absolute coords;
	int X = x * scale  + centerX;
	int X2 = x * scale + centerX;

	MoveToEx(hDC, X , centerY, NULL);
	LineTo(hDC, X2, centerY + Y);
}


//output sample to the graph
void GraphConsole::draw_sample(float * sample, int offset ){
	if (!sample) return;
	float* p = sample;
	int x = 0;
	for (int i = 0; i < asio_buffer_length; i++) {
		float val = *p++;
		draw_new_point(x++ + offset * asio_buffer_length, val);
		if (x*scale > client_width )
			break;
	}
};

