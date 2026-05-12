#pragma once

#include <Windows.h>
#include <iostream>

#define CRASH() do \
{ \
	int* ptr = nullptr; \
	*ptr = 100; \
} while(0)

#ifdef WIN64
template<typename T>
class CMemoryPool
{
public:
	CMemoryPool(bool _placement = false) {
		placement = _placement;
		Node* Padding = (Node*)malloc(sizeof(Node) + sizeof(T));
		if (Padding == nullptr)
			exit(1);
		padding = ((__int64)(&Padding->node) - (__int64)Padding) / sizeof(PVOID);
		free(Padding);
	};
	CMemoryPool(int _size, bool _placement = false)
	{
	}
	~CMemoryPool()
	{
		Node* pNode;
		Node* nextNode;
		pNode = (Node*)d_TopNode.u_Tag.Address;
		if (pNode == nullptr)
			return;
		while (1)
		{
			nextNode = pNode->next;
			free(pNode);
			if (nextNode == nullptr)
				break;
			pNode = nextNode;
		}
	};
	void SetPlacement(bool _placement) { placement = _placement; }

	T* Alloc();
	bool Free(T* data);
	DWORD GetAllocCount() { return AllocCnt; }
	DWORD GetFreeCnt() { return FreeCnt; }

private:

	struct Node
	{
		CMemoryPool<T>* ovcheck;
		T node;
		CMemoryPool<T>* uncheck;
		__int32 tag;
		Node* next;
	};
	struct DoubleCASNode
	{
		__int64 Address : 48;
		__int64 Tag : 16;
	};
	union DoubleCAS
	{
		__int64 TagAddress;
		DoubleCASNode u_Tag;
	};

	bool placement;
	size_t padding;
	__int16 UniqueValue = 0;

	DWORD AllocCnt = 0;
	DWORD FreeCnt = 0;

	DoubleCAS d_TopNode = { 0 };
};

template<typename T>
inline T* CMemoryPool<T>::Alloc()
{
	InterlockedIncrement(&AllocCnt);
	unsigned __int16 value = InterlockedIncrement16(&UniqueValue);

	Node* pNode;
	Node* nextTop;
	Node* retNode;

	DoubleCAS compareCAS;
	DoubleCAS nextCAS;
	while (1)
	{
		if (d_TopNode.TagAddress == 0)
		{
			pNode = (Node*)malloc(sizeof(Node) + sizeof(T));
			pNode->ovcheck = this;
			pNode->uncheck = this;

			pNode->next = nullptr;
			pNode->tag = value;

			if (placement)
			{
				new(&pNode->node) T;
			}
			retNode = pNode;
			break;
		}

		pNode = (Node*)d_TopNode.u_Tag.Address;
		if (pNode == nullptr)
			continue;

		compareCAS.u_Tag.Address = (__int64)pNode;
		compareCAS.u_Tag.Tag = pNode->tag;

		nextTop = pNode->next;
		if (nextTop == nullptr)
		{
			nextCAS.TagAddress = 0;
		}
		else
		{
			nextCAS.u_Tag.Address = (__int64)nextTop;
			nextCAS.u_Tag.Tag = nextTop->tag;
		}

		__int64 ret = InterlockedCompareExchange64((__int64*)&d_TopNode.TagAddress, (__int64)nextCAS.TagAddress, (__int64)compareCAS.TagAddress);
		if (ret == (__int64)compareCAS.TagAddress)
		{
			pNode->tag = value;
			retNode = pNode;
			break;
		}
	}
	return &retNode->node;
}

template<typename T>
inline bool CMemoryPool<T>::Free(T* data)
{
	unsigned __int16 value = InterlockedIncrement16(&UniqueValue);

	Node* pNode = (Node*)((PVOID*)data - padding);
	pNode->tag = value;

	Node* preTop;

	DoubleCAS compareCAS;
	DoubleCAS nextCAS;

	nextCAS.u_Tag.Address = (__int64)pNode;
	nextCAS.u_Tag.Tag = pNode->tag;

	if (pNode->ovcheck != this)
		CRASH();
	if (pNode->uncheck != this)
		CRASH();
	while (1)
	{
		preTop = (Node*)d_TopNode.u_Tag.Address;

		pNode->next = preTop;


		if (preTop == nullptr)
		{
			compareCAS.TagAddress = 0;
		}
		else
		{
			compareCAS.u_Tag.Address = (__int64)preTop;
			compareCAS.u_Tag.Tag = preTop->tag;
		}

		__int64 ret = InterlockedCompareExchange64((__int64*)&d_TopNode.TagAddress, (__int64)nextCAS.TagAddress, (__int64)compareCAS.TagAddress);

		if (ret == (__int64)compareCAS.TagAddress)
		{
			InterlockedIncrement(&FreeCnt);
			break;
		}
	}

	return true;
}


#elif WIN32
template<typename T>
class CMemoryPool
{
private:
	struct Node
	{
		CMemoryPool<T>* ovcheck;
		T node;
		CMemoryPool<T>* uncheck;
		__int32 tag;
		Node* next;
	};
	struct DoubleCASNode
	{
		__int32 Address;
		__int32 Tag;
	};
	union DoubleCAS
	{
		__int64 TagAddress;

		DoubleCASNode u_Tag;
	};

public:
	CMemoryPool(bool _placement = false) {
		placement = _placement;
		Node* Padding = (Node*)malloc(sizeof(Node) + sizeof(T));
		padding = ((__int32)(&Padding->node) - (__int32)Padding) / sizeof(PVOID);
		free(Padding);
	};
	CMemoryPool(int _size, bool _placement = false)
	{
	}
	~CMemoryPool()
	{
		Node* pNode;
		Node* nextNode;
		pNode = (Node*)d_TopNode.u_Tag.Address;
		if (pNode == nullptr)
			return;
		while (1)
		{
			nextNode = pNode->next;
			free(pNode);
			if (nextNode == nullptr)
				break;
			pNode = nextNode;
		}
	};
	void SetPlacement(bool _placement) { placement = _placement; }


	T* Alloc();
	bool Free(T* data);
	DWORD GetAllocCount() { return AllocCnt; }
	DWORD GetFreeCnt() { return FreeCnt; }

private:



	bool placement;

	int padding;

	unsigned int UniqueValue = 0;

	DWORD AllocCnt = 0;
	DWORD FreeCnt = 0;

	DoubleCAS d_TopNode = { 0 };
};

template<typename T>
inline T* CMemoryPool<T>::Alloc()
{
	InterlockedIncrement(&AllocCnt);
	unsigned int value = 0;
	Node* pNode;

	Node* nextTop;

	DoubleCAS compareCAS;
	DoubleCAS nextCAS;
	while (1)
	{
		if (d_TopNode.TagAddress == 0)
		{
			value = _InterlockedIncrement(&UniqueValue);
			pNode = (Node*)malloc(sizeof(Node) + sizeof(T));
			pNode->ovcheck = this;
			pNode->uncheck = this;

			pNode->next = nullptr;
			pNode->tag = value;

			if (placement)
			{
				new(&pNode->node) T;
			}
			break;
		}

		pNode = (Node*)d_TopNode.u_Tag.Address;
		if (pNode == nullptr)
			continue;

		compareCAS.u_Tag.Address = (__int32)pNode;
		compareCAS.u_Tag.Tag = pNode->tag;

		nextTop = pNode->next;
		if (nextTop == nullptr)
		{
			nextCAS.TagAddress = 0;
		}
		else
		{
			nextCAS.u_Tag.Address = (__int32)nextTop;
			nextCAS.u_Tag.Tag = nextTop->tag;
		}

		__int64 ret = InterlockedCompareExchange64((__int64*)&d_TopNode.TagAddress, (__int64)nextCAS.TagAddress, (__int64)compareCAS.TagAddress);
		if (ret == (__int64)compareCAS.TagAddress)
		{
			break;
		}
	}

	return &pNode->node;
}

template<typename T>
inline bool CMemoryPool<T>::Free(T* data)
{
	Node* pNode = (Node*)((PVOID*)data - padding);
	Node* preTop;

	DoubleCAS compareCAS;
	DoubleCAS nextCAS;

	if (pNode->ovcheck != this)
		__CRASH__();
	if (pNode->uncheck != this)
		__CRASH__();
	while (1)
	{
		preTop = (Node*)d_TopNode.u_Tag.Address;

		pNode->next = preTop;


		if (preTop == nullptr)
		{
			compareCAS.TagAddress = 0;
		}
		else
		{
			compareCAS.u_Tag.Address = (__int32)preTop;
			compareCAS.u_Tag.Tag = preTop->tag;
		}

		nextCAS.u_Tag.Address = (__int32)pNode;
		nextCAS.u_Tag.Tag = pNode->tag;

		__int64 ret = InterlockedCompareExchange64((__int64*)&d_TopNode.TagAddress, (__int64)nextCAS.TagAddress, (__int64)compareCAS.TagAddress);
		if (ret == (__int64)compareCAS.TagAddress)
		{
			InterlockedIncrement(&FreeCnt);
			break;
		}
	}

	return true;
}
#endif // _WIN64


template<typename T>
class CLOCKMemoryPool
{
public:
	CLOCKMemoryPool(bool _placement = false) { placement = _placement; InitializeCriticalSection(&cs); };
	CLOCKMemoryPool(int _size, bool _placement = false)
	{
	}
	~CLOCKMemoryPool()
	{
		Node* pNode = TopNode;
		Node* nextNode;
		if (pNode == nullptr)
			return;
		while (1)
		{
			nextNode = pNode->next;
			free(pNode);
			if (nextNode == nullptr)
				break;
			pNode = nextNode;
		}
	};

	T* Alloc();
	bool Free(T* data);
	DWORD GetAllocCount() { return AllocCnt; }
	DWORD GetFreeCnt() { return FreeCnt; }

private:

	struct Node
	{
		CLOCKMemoryPool<T>* ovcheck;
		T node;
		CLOCKMemoryPool<T>* uncheck;
		Node* next;
	};
	bool CAS(Node** Dest, Node* Exchange, Node* Src)
	{
		DWORD ret = InterlockedCompareExchange((DWORD*)Dest, (DWORD)Exchange, (DWORD)Src);

		if (ret == (DWORD)Src)
		{
			return true;
		}
		return false;
	}

	void Lock(const char* str = __builtin_FUNCTION()) { EnterCriticalSection(&cs);}
	void UnLock(const char* str = __builtin_FUNCTION()) { LeaveCriticalSection(&cs); }
	bool placement;

	DWORD AllocCnt = 0;
	DWORD FreeCnt = 0;

	Node* TopNode;
	CRITICAL_SECTION cs;
};

template<typename T>
inline T* CLOCKMemoryPool<T>::Alloc()
{
	Node* pNode;

	Lock();
	pNode = TopNode;
	if (TopNode == nullptr)
	{
		LeaveCriticalSection(&cs);
		InterlockedIncrement(&AllocCnt);
		pNode = (Node*)malloc(sizeof(Node) + sizeof(T));
		pNode->ovcheck = this;
		pNode->uncheck = this;
	
		pNode->next = nullptr;
	
		if (placement)
		{
			new(&pNode->node) T;
		}
		return &pNode->node;
	}

	TopNode = pNode->next;
	UnLock();

	

	return &pNode->node;
}

template<typename T>
inline bool CLOCKMemoryPool<T>::Free(T* data)
{
	Node* pNode = (Node*)((PVOID*)data - 1);

	if (pNode->ovcheck != this)
		CRASH();
	if (pNode->uncheck != this)
		CRASH();
	
	Lock();
	
	pNode->next = TopNode;
	TopNode = pNode;
	
	UnLock();

	return true;
}