#ifndef _CPUCLASS_H_
#define _CPUCLASS_H_

#include <pdh.h>
#pragma comment(lib, "pdh.lib")


class CpuClass {

	bool canReadCpu;
	HQUERY hQuery;
	HCOUNTER hCounter;
	unsigned long lastSampleTime;
	long cpuDataUsage;

public:
	CpuClass();
	CpuClass(const CpuClass&);
	~CpuClass();

	void Initialize();
	void Shutdown();
	void Frame();

	int GetCpuPercentage();
};
#endif