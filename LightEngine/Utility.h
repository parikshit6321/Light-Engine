#pragma once

#include <Windows.h>
#include <fstream>
#include <iostream>

void InitUtility();
float getTimeElapsed();
void log(const char* message);

__int64 countsPerSec;
float secondsPerCount;

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

float getTimeElapsed()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
	return curTime * secondsPerCount;
}

void log(const char* message)
{
	std::ofstream outFile;
	outFile.open("log.log", std::ios::app);
	outFile << message;
	outFile << "\n";
	outFile.close();
}