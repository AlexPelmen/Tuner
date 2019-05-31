#pragma once
#include <fstream>
#include <mutex>

#define LOG_FILE "log.txt"

class Logger
{
public:
	Logger();
	~Logger();

	std::ofstream fout;
	std::mutex lock_file;

	void log_fft(float* fft);
	void log_note(char* note);
};

