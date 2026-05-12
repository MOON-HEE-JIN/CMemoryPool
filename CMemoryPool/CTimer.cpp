#include "CTimer.h"
#include <profileapi.h>
#include <iostream>

CTimer::CTimer()
{
	QueryPerformanceFrequency(&freq);
}

CTimer::~CTimer()
{
}

void CTimer::StartTimer()
{
	QueryPerformanceCounter(&start);
}

void CTimer::EndTimer()
{
	QueryPerformanceCounter(&end);

	DeltaTime = (end.QuadPart - start.QuadPart)/ (float)freq.QuadPart * 1000;
	++Count;
	total_DeltaTime += DeltaTime;
	avg_DeltaTime = total_DeltaTime / Count;
	max_DeltaTime = (max_DeltaTime > DeltaTime) ? max_DeltaTime : DeltaTime;
	min_DeltaTime = (min_DeltaTime > DeltaTime) ? DeltaTime : min_DeltaTime;
}

void CTimer::ResultTimer(const char* str)
{
	std::cout << str << " Thread ID : " << ThreadID << "\tavg_DeltaTime : " << avg_DeltaTime << "\tCount : " << Count << std::endl;
}

void CTimer::TotalTimer(const char* str)
{
	std::cout << str << " Total Time : " << total_DeltaTime << std::endl;
}

