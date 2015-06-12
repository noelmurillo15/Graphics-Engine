#ifndef _TIMERCLASS_H_
#define _TIMERCLASS_H_

#include <windows.h>
#define WIN32_LEAN_AND_MEAN


class TimerClass {

	INT64 frequency;
	INT64 startTime;

	float ticksPerMs;
	float frameTime;
	float elapsedTime;

public:

	TimerClass();
	TimerClass(const TimerClass&);
	~TimerClass();

	bool Initialize();
	void Frame();

	float GetTime();
	int GetElapsedTime();
};
#endif