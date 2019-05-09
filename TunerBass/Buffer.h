#include <deque>
#include <mutex>
#pragma once

//Common buffer

class Buffer {
public:
	
	std::deque< int* > data;		//queue of samples
	std::mutex _mutex;
	int asio_buffer_length;
	int b_max_sample_num;	//num of buffers, which would be included in the queue
	int b_rate;				//sample rate
	int b_time;				//time of the buffer
	
	GraphConsole *Graph = new GraphConsole;

	Buffer() {}
	//set buffer length and samples num
	Buffer(int sample_rate, int time, int buffer_length) {

		b_rate = sample_rate;
		b_time = time;
		asio_buffer_length = buffer_length;

		b_max_sample_num = floor((b_time*b_rate) / asio_buffer_length) + 1;	//samples num		
		init();  //fill the buffer with zeros
	}	

	//Setters
	void set_sample_rate( int sample_rate ) { b_rate = sample_rate;	}
	void set_buffer_length(int buffer_length ) { asio_buffer_length = buffer_length; }
	void set_time(int time) { b_time = time; }

	//counts a number of samples in the buffer
	void count_max_sample_num() {
		b_max_sample_num = floor((b_time*b_rate) / asio_buffer_length) + 1;
	}

	//init buffer: create samples and fill'em with zeros
	void init() {
		count_max_sample_num();
		for (int i = 0; i < b_max_sample_num; i++) {
			int* sample = new int[ asio_buffer_length ];

			for (int sam_i = 0; sam_i < asio_buffer_length; sam_i++)	//fill with zeros
				sample[ sam_i ] = 0;

			data.push_front(sample);
		}
	}
		
	//adds new sample to the buffer and remove one from the end
	void sample_add(int *sample) {

		_mutex.lock();

		//remove
		int* sample_to_remove = data.back();
		delete sample_to_remove;			
		data.pop_back();

		//add new sample
		int* elem = new int[ asio_buffer_length ];
		memcpy( elem, sample, asio_buffer_length );
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
			Graph->draw_sample(sample, i );
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
};