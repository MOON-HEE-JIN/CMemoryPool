#pragma once

#include <Windows.h>
#include <float.h>

class CTimer
{
public:
	CTimer();
	~CTimer();

	void StartTimer();
	void EndTimer();
	void ResultTimer(const char* str);
	void TotalTimer(const char* str);

	int ThreadID = 0;
private:
	LARGE_INTEGER start, end;
	LARGE_INTEGER freq;


	float DeltaTime = 0;
	unsigned int Count = 0;
	float total_DeltaTime = 0;
	float avg_DeltaTime = 0;
	float max_DeltaTime = 0;
	float min_DeltaTime = FLT_MAX;
};

