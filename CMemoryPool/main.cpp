// CMemoryPool.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "TestThread.h"
#include <exception>
#include <Windows.h>
#include "CPacketRingBuffer.h"
#include "CLockFreeQueue.h"
#include "CHazardPointer_MemoryPool.h"


int main()
{
    //LockFreeQueueTestStart();

    //LockFreeQueueTestEnd();

    MemoryPoolTestStart();

    MemoryPoolTestEnd();
    
}
