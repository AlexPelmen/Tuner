#pragma once
#include "Graph.h"
#include <iostream>

using namespace std;

GraphConsole::GraphConsole() {
	
	//Handles
	hWnd = GetConsoleWindow();
	GetClientRect(hWnd, &wndRect );
	hDC = GetDC( hWnd );

	//params	
	width = wndRect.right - wndRect.left - paddingX * 2;
	height = wndRect.bottom - wndRect.top - paddingY * 2;
	centerX = paddingX;
	centerY = paddingY + height / 2;
	
	//Pen and brush
	WhitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	BlackBrush = CreateSolidBrush(RGB(0, 0, 0));

	scale = asio_buffer_length / width;
};

void GraphConsole::set_asio_buffer_length(int buffer_length) {
	asio_buffer_length = buffer_length;
	scale = (float)width/ asio_buffer_length;
}


void GraphConsole::draw_axis() {
	SelectObject(hDC, WhitePen);
	//Horizontal
	MoveToEx(hDC, centerX, centerY, NULL);
	LineTo(hDC, width + paddingX - 1, centerY);
	//Vertical
	MoveToEx(hDC, centerX, centerY, NULL);
	LineTo(hDC, centerX, centerY - height / 2);
	MoveToEx(hDC, centerX, centerY, NULL);
	LineTo(hDC, centerX, centerY + height / 2);
};

void GraphConsole::clear() {
	SelectObject(hDC, BlackBrush);
	Rectangle(hDC, paddingX, paddingY, wndRect.right - paddingX, wndRect.bottom - paddingY + 1 );
};

void GraphConsole::draw_new_point(float x, float y) {
	SelectObject(hDC, WhitePen);
	float relVal = y / (float)INT_MAX / 2;	//normalize
	int Y = round(height * relVal );		//get val

	//to absolute coords;
	int X = x * scale  + centerX;
	int X2 = x * scale + centerX;

	MoveToEx(hDC, X , centerY, NULL);
	LineTo(hDC, X2, centerY + Y);
}


//output sample to the graph
void GraphConsole::draw_sample(int * sample, int offset ){
	if (!sample) return;
	int* p = sample;
	int x = 0;
	for (int i = 0; i < asio_buffer_length; i++) {
		draw_new_point(x++ + offset * asio_buffer_length, *p++);
		if (x*scale > width)
			break;
	}
};

