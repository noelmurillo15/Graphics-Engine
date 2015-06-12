#include "FPSClass.h"

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")


FPSClass::FPSClass() {
}

FPSClass::FPSClass(const FPSClass& other) {
}

FPSClass::~FPSClass() {
}

void FPSClass::Initialize() {
	fps = 0;
	count = 0;
	startTime = timeGetTime();
	return;
}

void FPSClass::Frame() {
	count++;

	if (timeGetTime() >= (startTime + 1000)) {
		fps = count;
		count = 0;
		startTime = timeGetTime();
	}
}

int FPSClass::GetFps() {
	return fps;
}