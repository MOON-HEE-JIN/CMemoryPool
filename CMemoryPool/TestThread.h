#pragma once

#define MAX_THREAD_COUNT 8 // 최대 Thread 생성 개수
#define MAX_THREAD_MEMORYPOOL_COUNT 500 // Thread 당 메모리풀 할당 개수



struct st_TestData
{
	char dumpdata[2048];
	int Data=0;	//Thread 당 고유 ID
	int Cnt=0;	//사용 횟수
};

//
// MemoryPool Test Func
//
void MemoryPoolTestStart();
void MemoryPoolTestEnd();

unsigned __stdcall CMemoryPoolTestMoniter(void* pParam);
unsigned __stdcall CMemoryPoolTest(void* pParam);
unsigned __stdcall HazardPoolTest(void* pParam);
unsigned __stdcall LockMemoryPoolTest(void* pParam);
unsigned __stdcall NewAllocTest(void* pParam);
unsigned __stdcall ManagerPoolTest(void* pParam);
//
// LockFreeQueue Test Func
//
void LockFreeQueueTestStart();
void LockFreeQueueTestEnd();

unsigned __stdcall CLockFreeQueueTestMoniter(void* pParam);
unsigned __stdcall CLockFreeQueueTest(void* pParam);
