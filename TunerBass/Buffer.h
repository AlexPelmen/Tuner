/*#include <deque>
#include <mutex>
#pragma once

//Common buffer

class Buffer {
public:
	
	std::deque< int* > data;		//queue of samples
	std::mutex _mutex;
	int b_max_sample_num;	//num of buffers, which would be included in the queue
	int pixels_per_sample;
	
	GraphConsole *Graph = new GraphConsole;

	Buffer() {}
	//set buffer length and samples num
	Buffer(int sample_rate, int time, int asio_buffer_length ) {
		b_max_sample_num = floor(( time*sample_rate) / asio_buffer_length) + 1;	//samples num	
		Graph->b_max_sample_num = b_max_sample_num;
		init();  //fill the buffer with zeros
	}	

	//counts a number of samples in the buffer
	void set_max_sample_num(int sample_rate, int time, int asio_buffer_length, int buffer_length ) {
		b_max_sample_num = floor((time*sample_rate) / asio_buffer_length) + 1;
	}

	//init buffer: create samples and fill'em with zeros
	void init() {
		for (int i = 0; i < b_max_sample_num; i++) {
			int* sample = new int[ pixels_per_sample ];

			for (int sam_i = 0; sam_i < pixels_per_sample; sam_i++)	//fill with zeros
				sample[ sam_i ] = 0;

			data.push_front(sample);
		}
	}
		
	//adds new sample to the buffer and remove one from the end
	void sample_add(int *sample) {

		//remove
		int* sample_to_remove = data.back();
		delete sample_to_remove;
		data.pop_back();

		int* elem;

		if (pixels_per_sample >= 0)
		{
			//several pixels per one sample
			elem = new int[pixels_per_sample];
			elem = Graph->get_pixels_from_sample(elem, sample);

		}
		else {
			//several samples per one pixel


		}

		_mutex.lock();		

		//add new sample
		int* elem = new int[ pixels_per_sample ];
		elem = Graph->get_pixels_from_sample( elem, sample );
		data.push_front( elem );

		_mutex.unlock();
	}

	

	//display buffer's inner in console with graph
	void buffer_draw() {
		int* sample = data.front();
		Graph->clear();
		Graph->draw_axis();

		_mutex.lock();

		//draw samples
		for (int i = 0; i < b_max_sample_num; i++ ) {
			Graph->draw_sample(sample );
			sample = data.at(i);
		}

		_mutex.unlock();
	}

	//clear buffer (just fill it with zeros)
	void buffer_clear() {
		int* sample;
		for (int i = 0; i < b_max_sample_num; i++) {
			sample = data.front();
			for (int sam_i = 0; sam_i < asio_buffer_length; sam_i++)
				sample[sam_i] = 0;
		}
	}

	//release memory
	~Buffer() {
		int* sample;
		for (int i = 0; i < b_max_sample_num; i++ ) {
			sample = data.back();
			delete sample;
			data.pop_back();
		}
		std::cout << "Big buffer deleted" << std::endl;
	}
};*/