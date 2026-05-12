#include "TestThread.h"
#include <process.h>
//#include <Windows.h>
#include "CTimer.h"
#include "CLockFreeQueue.h"
#include "CLockFreeStack.h"
#include "CHazardPointer_MemoryPool.h"
#include "CMomoryPool_InterLockSList.h"

CMemoryPool<st_TestData> g_MemoryPool;// tag 를 이용해서 비교를 한다고 해도 ABA 가능성 존재 특히 alloc 과 free 를 자주 빠르게 한다면 문제 발생 가능성 증가
CHMemoryPool<st_TestData> g_HazardPool;
CLOCKMemoryPool<st_TestData> g_LockMemoryPool;
CLockFreeStack<st_TestData> g_LF_Stack;

CMemoryPoolManager<st_TestData> g_MemoryManager;
MemoryPool<st_TestData> g_MemoryTLSpool(&g_MemoryManager);


unsigned int g_MemoryTimerIndex;
CTimer* g_MemoryPoolAllocTimer[MAX_THREAD_COUNT];
CTimer* g_MemoryPoolFreeTimer[MAX_THREAD_COUNT];

unsigned int g_HazardTimerIndex;
CTimer* g_HazardPoolAllocTimer[MAX_THREAD_COUNT];
CTimer* g_HazardPoolFreeTimer[MAX_THREAD_COUNT];

unsigned int g_LockMemoryTimerIndex;
CTimer* g_LockMemoryPoolAllocTimer[MAX_THREAD_COUNT];
CTimer* g_LockMemoryPoolFreeTimer[MAX_THREAD_COUNT];

unsigned int g_NewkMemoryTimerIndex;
CTimer* g_NewMemoryPoolAllocTimer[MAX_THREAD_COUNT];
CTimer* g_NewMemoryPoolFreeTimer[MAX_THREAD_COUNT];

unsigned int g_ManagerMemoryTimerIndex;
CTimer* g_ManagerMemoryAllocTimer[MAX_THREAD_COUNT];
CTimer* g_ManagerMemoryFreeTimer[MAX_THREAD_COUNT];


HANDLE g_hThread[MAX_THREAD_COUNT + 1];
HANDLE g_hLockThread[MAX_THREAD_COUNT + 1];
HANDLE g_hNewThread[MAX_THREAD_COUNT + 1];
HANDLE g_hManagerThread[MAX_THREAD_COUNT + 1];

bool g_MemoryPoolTestBool;



CLockFreeQueue<st_TestData*> g_LF_Queue;

bool g_LockFreeQueueTestBool;

HANDLE g_hQueueThread[MAX_THREAD_COUNT+1];




void MemoryPoolTestStart()
{
	g_MemoryPoolTestBool = true;
	for (int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		//LockFree
		//g_MemoryPoolAllocTimer[i] = new CTimer;
		//g_MemoryPoolFreeTimer[i] = new CTimer;
		//g_hThread[i] = (HANDLE)_beginthreadex(NULL, 0, CMemoryPoolTest, 0, 0, 0);

		//Harzard
		//g_HazardPoolAllocTimer[i] = new CTimer;
		//g_HazardPoolFreeTimer[i] = new CTimer;
		//g_hThread[i] = (HANDLE)_beginthreadex(NULL, 0, HazardPoolTest, 0, 0, 0);

		//Lock
		//g_LockMemoryPoolAllocTimer[i] = new CTimer;
		//g_LockMemoryPoolFreeTimer[i] = new CTimer;
		//g_hLockThread[i] = (HANDLE)_beginthreadex(NULL, 0, LockMemoryPoolTest, 0, 0, 0);

		//New
		//g_NewMemoryPoolAllocTimer[i] = new CTimer;
		//g_NewMemoryPoolFreeTimer[i] = new CTimer;
		//g_hNewThread[i] = (HANDLE)_beginthreadex(NULL, 0, NewAllocTest, 0, 0, 0);

		g_ManagerMemoryAllocTimer[i] = new CTimer;
		g_ManagerMemoryFreeTimer[i] = new CTimer;
		g_hThread[i] = (HANDLE)_beginthreadex(NULL, 0, ManagerPoolTest, 0, 0, 0);

	}
	g_hThread[MAX_THREAD_COUNT] = (HANDLE)_beginthreadex(NULL, 0, CMemoryPoolTestMoniter, 0, 0, 0);
	
}

void MemoryPoolTestEnd()
{
	int ret = WaitForMultipleObjects(MAX_THREAD_COUNT+1, g_hThread, true, INFINITE);
	WaitForMultipleObjects(MAX_THREAD_COUNT, g_hLockThread, true, INFINITE);
	WaitForMultipleObjects(MAX_THREAD_COUNT, g_hNewThread, true, INFINITE);
}

unsigned __stdcall CMemoryPoolTestMoniter(void* pParam)
{
	while (g_MemoryPoolTestBool)
	{
		std::cout << "=====================================================\n";
		//std::cout << "총 생성된 Thread 개수 : " << MAX_THREAD_COUNT << std::endl;
		std::cout << "Memory Pool 설정된 메모리 개수 : " << MAX_THREAD_MEMORYPOOL_COUNT * MAX_THREAD_COUNT << std::endl;
		//std::cout << "Memory Pool 총 할당된 메모리 개수 : " << g_MemoryPool.GetAllocCount() << std::endl;
		std::cout << std::endl;


		//g_MemoryPoolAllocTimer[0]->ResultTimer("MemoryPool Alloc Timer ");
		//g_MemoryPoolFreeTimer[0]->ResultTimer("MemoryPool Free Timer ");
		
		//std::cout << "Memory Pool 총 할당/해제 된 메모리 개수 : " << g_HazardPool.GetAllocCount() << "/" << g_HazardPool.GetFreeCount() << std::endl;
		//g_HazardPoolAllocTimer[0]->ResultTimer("Hazard Alloc Timer ");
		//g_HazardPoolFreeTimer[0]->ResultTimer("Hazard Free Timer ");
		
		//std::cout << std::endl;
		//g_LockMemoryPoolAllocTimer[0]->ResultTimer("Lock Alloc Timer ");
		//g_LockMemoryPoolFreeTimer[0]->ResultTimer("Lock Free Timer ");
		
		//g_NewMemoryPoolAllocTimer[0]->ResultTimer("NEW Alloc Timer ");
		//g_NewMemoryPoolFreeTimer[0]->ResultTimer("FREE Free Timer ");
		std::cout << "=====================================================\n";
		/*for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
		}*/

		g_ManagerMemoryAllocTimer[0]->ResultTimer("Lock Alloc Timer ");
		g_ManagerMemoryFreeTimer[0]->ResultTimer("Lock Free Timer ");

		if (GetAsyncKeyState('p') || GetAsyncKeyState('P'))
			g_MemoryPoolTestBool = false;

		Sleep(1000);
	}

	g_MemoryPoolAllocTimer[0]->TotalTimer("LockFree Alloc Timer ");
	g_MemoryPoolFreeTimer[0]->TotalTimer("LockFree Free Timer ");

	//g_LockMemoryPoolAllocTimer[0]->TotalTimer("Lock Alloc Timer ");
	//g_LockMemoryPoolFreeTimer[0]->TotalTimer("Lock Free Timer ");

	//g_NewMemoryPoolAllocTimer[0]->TotalTimer("Lock Alloc Timer ");
	//g_NewMemoryPoolFreeTimer[0]->TotalTimer("Lock Free Timer ");

	return 1;
}


unsigned WINAPI CMemoryPoolTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	int TimerIndex = InterlockedIncrement(&g_MemoryTimerIndex) - 1;
	CTimer* alloc_timer = g_MemoryPoolAllocTimer[TimerIndex];
	CTimer* free_timer = g_MemoryPoolFreeTimer[TimerIndex];

	alloc_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	free_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = g_MemoryPool.Alloc();
	}
	//Sleep(2);
	while (g_MemoryPoolTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨"  << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread())+1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			free_timer->StartTimer();
			g_MemoryPool.Free(pTestData[i]);
			free_timer->EndTimer();
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			alloc_timer->StartTimer();
			pTestData[i] = g_MemoryPool.Alloc();
			alloc_timer->EndTimer();
		}
		//Sleep(2);
	}

	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		g_MemoryPool.Free(pTestData[i]);
	}

	return 1;
}

unsigned __stdcall HazardPoolTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	int TimerIndex = InterlockedIncrement(&g_HazardTimerIndex) - 1;
	CTimer* alloc_timer = g_HazardPoolAllocTimer[TimerIndex];
	CTimer* free_timer = g_HazardPoolFreeTimer[TimerIndex];

	alloc_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	free_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = g_HazardPool.Alloc();
	}
	//Sleep(2);
	while (g_MemoryPoolTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨" << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread()) + 1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			free_timer->StartTimer();
			g_HazardPool.Free(pTestData[i]);
			free_timer->EndTimer();
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			alloc_timer->StartTimer();
			pTestData[i] = g_HazardPool.Alloc();
			alloc_timer->EndTimer();
		}
		//Sleep(2);
	}

	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		g_HazardPool.Free(pTestData[i]);
	}

	return 1;
}

unsigned __stdcall LockMemoryPoolTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	int TimerIndex = InterlockedIncrement(&g_LockMemoryTimerIndex) - 1;
	CTimer* alloc_timer = g_LockMemoryPoolAllocTimer[TimerIndex];
	CTimer* free_timer = g_LockMemoryPoolFreeTimer[TimerIndex];

	alloc_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	free_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = g_LockMemoryPool.Alloc();
	}
	//Sleep(2);
	while (g_MemoryPoolTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨" << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread()) + 1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			free_timer->StartTimer();
			g_LockMemoryPool.Free(pTestData[i]);
			free_timer->EndTimer();
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			alloc_timer->StartTimer();
			pTestData[i] = g_LockMemoryPool.Alloc();
			alloc_timer->EndTimer();
		}
		//Sleep(2);
	}

	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		g_LockMemoryPool.Free(pTestData[i]);
	}

	return 1;
}

unsigned __stdcall NewAllocTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	int TimerIndex = InterlockedIncrement(&g_NewkMemoryTimerIndex) - 1;
	CTimer* alloc_timer = g_NewMemoryPoolAllocTimer[TimerIndex];
	CTimer* free_timer = g_NewMemoryPoolFreeTimer[TimerIndex];

	alloc_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	free_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = new st_TestData;
	}
	//Sleep(2);
	while (g_MemoryPoolTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨" << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread()) + 1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			free_timer->StartTimer();
			delete pTestData[i];
			free_timer->EndTimer();
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			alloc_timer->StartTimer();
			pTestData[i] = new st_TestData;
			alloc_timer->EndTimer();
		}
		//Sleep(2);
	}

	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		delete pTestData[i];
	}

	return 1;
}

unsigned __stdcall ManagerPoolTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	int TimerIndex = InterlockedIncrement(&g_ManagerMemoryTimerIndex) - 1;
	CTimer* alloc_timer = g_ManagerMemoryAllocTimer[TimerIndex];
	CTimer* free_timer = g_ManagerMemoryFreeTimer[TimerIndex];

	alloc_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	free_timer->ThreadID = (DWORD)GetThreadId(GetCurrentThread());

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = g_MemoryTLSpool.Alloc();
	}
	//Sleep(2);
	while (g_MemoryPoolTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨" << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread()) + 1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			free_timer->StartTimer();
			g_MemoryTLSpool.Free(pTestData[i]);
			free_timer->EndTimer();
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			
			alloc_timer->StartTimer();
			pTestData[i] = g_MemoryTLSpool.Alloc();
			alloc_timer->EndTimer();
		}
		//Sleep(2);
	}

	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		delete pTestData[i];
	}

	return 1;
}

void LockFreeQueueTestStart()
{
	g_LockFreeQueueTestBool = true;
	for (int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		g_hQueueThread[i] = (HANDLE)_beginthreadex(NULL, 0, CLockFreeQueueTest, 0, 0, 0);
	}
	g_hQueueThread[MAX_THREAD_COUNT] = (HANDLE)_beginthreadex(NULL, 0, CLockFreeQueueTestMoniter, 0, 0, 0);
}

void LockFreeQueueTestEnd()
{
	WaitForMultipleObjects(MAX_THREAD_COUNT , g_hQueueThread, true, INFINITE);
}

unsigned __stdcall CLockFreeQueueTestMoniter(void* pParam)
{
	while (g_LockFreeQueueTestBool)
	{
		std::cout << "=====================================================\n";
		//std::cout << "총 생성된 Thread 개수 : " << MAX_THREAD_COUNT << std::endl;
		std::cout << "LockFreeQueue 설정된 메모리 개수 : " << MAX_THREAD_MEMORYPOOL_COUNT * MAX_THREAD_COUNT << std::endl;

		// - 1 이유는 LockFreeQueue 는 Dummy 노드를 한번 생성하기에 새로 생성한 노드의 개수만 보기 위해 뺐다
		std::cout << "LockFreeQueue 총 할당된 메모리 개수 : " << g_LF_Queue.GetMemoryPoolSize() - 1 << std::endl;
		std::cout << "=====================================================\n";
		/*for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
		}*/


		if (GetAsyncKeyState('p') || GetAsyncKeyState('P'))
			g_LockFreeQueueTestBool = false;

		Sleep(1000);
	}
	return 1;
}

unsigned __stdcall CLockFreeQueueTest(void* pParam)
{
	st_TestData* pTestData[MAX_THREAD_MEMORYPOOL_COUNT];

	//미리 정해진 횟수 만큼 메모리풀 할당
	//테스트에 있어 할당된 메모리 개수가 증가 되는것을 확인 하기 위함 (증가되면 안되는거)
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		pTestData[i] = new st_TestData;
	}
	for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
	{
		g_LF_Queue.Enqueue(pTestData[i]);
	}
	Sleep(2);
	while (g_LockFreeQueueTestBool)
	{
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i] = g_LF_Queue.Dequeue();
		}

		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			pTestData[i]->Data = (DWORD)GetThreadId(GetCurrentThread());
			pTestData[i]->Cnt = 0;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != (DWORD)GetThreadId(GetCurrentThread()) || pTestData[i]->Cnt != 0)
				std::cout << "이미 사용중인 메모리가 할당 됨" << std::endl;
		}

		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			InterlockedIncrement((DWORD*)&pTestData[i]->Data);
			InterlockedIncrement((DWORD*)&pTestData[i]->Cnt);
		}
		Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			if (pTestData[i]->Data != ((DWORD)GetThreadId(GetCurrentThread()) + 1) || pTestData[i]->Cnt != 1)
				std::cout << "할당된 메모리 두군데 이상에서 사용중 %d :" << pTestData[i]->Cnt << std::endl;
		}
		//Sleep(2);
		for (int i = 0; i < MAX_THREAD_MEMORYPOOL_COUNT; i++)
		{
			g_LF_Queue.Enqueue(pTestData[i]);
		}
		//Sleep(2);
	}
	return 0;
}


