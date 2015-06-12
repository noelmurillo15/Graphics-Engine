#include "cpuclass.h"


CpuClass::CpuClass() {
}

CpuClass::CpuClass(const CpuClass& other) {
}

CpuClass::~CpuClass() {
}

void CpuClass::Initialize() {

	//	Kinda like an HRESULT / used for starting pdh members
	PDH_STATUS status;

	canReadCpu = true;

	// Create a query object to poll cpu usage
	status = PdhOpenQuery(NULL, 0, &hQuery);
	if (status != ERROR_SUCCESS)	//	safety check
		canReadCpu = false;	

	// Set query object to poll all cpus in the system.
	status = PdhAddCounter(hQuery, TEXT("\\Processor(_Total)\\% processor time"), 0, &hCounter);
	if (status != ERROR_SUCCESS)	//	safety check
		canReadCpu = false;
	
	lastSampleTime = GetTickCount();
	cpuDataUsage = 0;
	return;
}

void CpuClass::Shutdown() {
	if (canReadCpu)	//	if active, shut down pdh query
		PdhCloseQuery(hQuery);	
	return;
}

void CpuClass::Frame() {
	PDH_FMT_COUNTERVALUE value;

	if (canReadCpu) {
		if ((lastSampleTime + 1000) < GetTickCount()) {
			lastSampleTime = GetTickCount();

			//	Collect query data 
			PdhCollectQueryData(hQuery);

			//	We want the longvalue format
			PdhGetFormattedCounterValue(hCounter, PDH_FMT_LONG, NULL, &value);
			cpuDataUsage = value.longValue;
		}
	}
	return;
}

int CpuClass::GetCpuPercentage() {
	int usage;

	if (canReadCpu)	//	cast cpu usage to a readable %
		usage = (int)cpuDataUsage;	
	else
		usage = 0;
	
	return usage;
}