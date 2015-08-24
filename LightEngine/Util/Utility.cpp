#include "Utility.h"

// Variables relevant for timer.
__int64 countsPerSec;
float secondsPerCount;

/*
	Initializes the different components of
	the Utility section : Timer and Logger.
*/
void InitUtility()
{
	// Initialize Timer.
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secondsPerCount = 1.0f / countsPerSec;

	// Initialize Logger.
	std::ofstream outFile;
	outFile.open("log.log", std::ios::out | std::ios::trunc);
	outFile.close();
}

/*
	Function to get the elapsed time in seconds
	by using the performance counter of the CPU.
*/
float getTimeElapsed()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
	return curTime * secondsPerCount;
}

/*
	Function to log messages to the log file
	for debugging purposes.
*/
void log(const char* message)
{
	std::ofstream outFile;
	outFile.open("log.log", std::ios::app);
	outFile << message;
	outFile << "\n";
	outFile.close();
}