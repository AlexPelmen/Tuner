#pragma once

class Analize {
public:
	int asio_buffer_length;

	Analize() {}
	Analize( int buffer_length ) {
		asio_buffer_length = buffer_length;
	}	

	void set_buffer_length( int buffer_length ) {
		asio_buffer_length = buffer_length;		
	}

	int get_maximum_index(float* sample) {	
		if (!asio_buffer_length)	//error
			return -1;

		float cur_max = 0;
		int cur_index_of_max = 0;
		for (int i = 0; i < asio_buffer_length/2; i++) {
			if (cur_max < sample[i]) {
				cur_max = sample[i];
				cur_index_of_max = i;
			}
		}
		return cur_index_of_max;
	}
};