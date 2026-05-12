#include "CIOCriticalSectionDebug.h"
#include <Windows.h>

struct st_CIO_CS_DEBUG
{
	int LockCount;
	unsigned int ThreadID;
	char name[256];
};

st_CIO_CS_DEBUG g_CIO_DEBUG_Array[100];
DWORD g_CIO_DEBUG_Index;

st_CIO_CS_DEBUG g_CCritMgr_DEBUG_Array[100];
DWORD g_CCritMgr_Index;

void CIO_Insert(int _index, const char* str = __builtin_FUNCTION())
{
	g_CIO_DEBUG_Array[_index].LockCount++;
	g_CIO_DEBUG_Array[_index].ThreadID = (DWORD)GetThreadId(GetCurrentThread());
	strcpy_s(g_CIO_DEBUG_Array[_index].name, 256, str);
}

void CIO_Earse(int _index)
{
	g_CIO_DEBUG_Array[_index].LockCount--;
}


void CMgr_Insert()
{

}

void CMgr_Earse()
{

}
