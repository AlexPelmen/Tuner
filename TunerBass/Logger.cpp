#include "Logger.h"
#include "ProcConfig.h"



Logger::Logger()
{
	fout.open( LOG_FILE );
}
Logger::~Logger()
{
	fout.close();
}


void Logger::log_fft(float * fft)
{
	lock_file.lock();

	for (int i = 0; i < FFT_LEN; i++) {
		fout << fft[i] << " ";
	}
	fout << std::endl;
	fout << std::endl;
	fout << std::endl;

	lock_file.unlock();
}

void Logger::log_note(char * note)
{
	lock_file.lock();

	fout << "note: " << note << "\t";

	lock_file.unlock();
}
