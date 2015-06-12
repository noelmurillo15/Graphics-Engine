#ifndef _FPSCLASS_H_
#define _FPSCLASS_H_


class FPSClass {

	int fps, count;
	unsigned long startTime;

public:

	FPSClass();
	FPSClass(const FPSClass&);
	~FPSClass();

	void Initialize();
	void Frame();
	int GetFps();
};
#endif