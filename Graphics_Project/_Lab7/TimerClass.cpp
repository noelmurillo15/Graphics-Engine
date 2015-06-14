#include "TimerClass.h"


TimerClass::TimerClass() {
}

TimerClass::TimerClass(const TimerClass& other) {
}

TimerClass::~TimerClass() {
}

bool TimerClass::Initialize() {
	// Check to see if this system supports high performance timers.
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)
		return false;	

	// Find out how many times the frequency counter ticks every millisecond.
	ticksPerMs = (float)(frequency / 1000);
	elapsedTime = 0;

	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	return true;
}

void TimerClass::Frame() {
	INT64 currentTime;
	float timeDifference;

	QueryPerformanceCounter((LARGE_INTEGER*)& currentTime);

	timeDifference = (float)(currentTime - startTime);

	//	time between 2 calls
	frameTime = timeDifference / ticksPerMs;

	//	total time program has been running
	elapsedTime += (frameTime / 1000.0f);

	startTime = currentTime;

	return;
}

float TimerClass::GetTime() {
	return frameTime / 1000.0f;
}

int TimerClass::GetElapsedTime() {
	return (int)elapsedTime;
}