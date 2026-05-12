#pragma once
#include <Windows.h>
#include "CMemoryPool.h"

#ifdef WIN64
template<typename T>

class CLockFreeStack
{
public:
	CLockFreeStack() { /*Top.data = 0; Top.UniqueAddress = 0; Top.next = nullptr;*/ };
	~CLockFreeStack() {};

private:
	struct Node
	{
		T data;
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
	CMemoryPool<Node> LockFreePool;

	DoubleCAS TopNode;

	DWORD Cnt = 0;
	short UniqueValue;


public:
	int GetMemoryPoolSize() { return LockFreePool.GetAllocCount(); }
	void Push(T _data)
	{
		short value = InterlockedIncrement16(&UniqueValue);

		Node* nNode = LockFreePool.Alloc();

		nNode->data = _data;
		nNode->tag = value;

		Node* preTop;

		DoubleCAS preTopNode;
		DoubleCAS newTopNode;

		while (1)
		{
			preTop = (Node*)TopNode.u_Tag.Address;
			nNode->next = preTop;

			preTopNode.u_Tag.Address = (__int64)preTop;
			preTopNode.u_Tag.Tag = preTop->tag;

			newTopNode.u_Tag.Address = (__int64)nNode;
			newTopNode.u_Tag.Tag = nNode->tag;

			__int64 ret = InterlockedCompareExchange64((__int64*)&TopNode.TagAddress, newTopNode.TagAddress, preTopNode.TagAddress);

			if (ret == preTopNode.TagNode)
			{
				InterlockedIncrement(&Cnt);
				break;
			}
		}
	}

	T Pop()
	{
		Node* popNode;
		Node* nextTop;

		DoubleCAS popTopNode;
		DoubleCAS nextTopNode;

		while (1)
		{
			if (TopNode.TagAddrss == nullptr)
				break;

			popNode = (Node*)TopNode.u_Tag.Address;
			nextTop = popNode.next;

			popTopNode.u_Tag.Address = (__int64)popNode;
			popTopNode.u_Tag.Tag = popNode.tag;

			nextTopNode.u_Tag.Address = (__int64)nextTop;
			nextTopNode.u_Tag.Tag = nextTop.tag;

			__int64 ret = InterlockedCompareExchange64((__int64*)&TopNode.TagAddress, nextTopNode.TagAddress, popTopNode.TagAddress);
			if (ret == popTopNode.TagAddress)
			{
				T data = popNode->data;

				LockFreePool.Free(popNode);
				return data;
			}
		}
		return nullptr;
	}
};

#elif WIN32

template<typename T>

class CLockFreeStack
{
public:
	CLockFreeStack() { /*Top.data = 0; Top.UniqueAddress = 0; Top.next = nullptr;*/ };
	~CLockFreeStack() {};

private:
	struct Node
	{
		T data;
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
	CMemoryPool<Node> LockFreePool;
	
	DoubleCAS TopNode;

	DWORD Cnt = 0;
	short UniqueValue;


public:
	int GetMemoryPoolSize() { return LockFreePool.GetAllocCount(); }
	void Push(T _data)
	{
		short value = InterlockedIncrement16(&UniqueValue);
		
		Node* nNode = LockFreePool.Alloc();
		
		nNode->data = _data;
		nNode->tag = value;

		Node* preTop;

		DoubleCAS preTopNode;
		DoubleCAS newTopNode;

		while (1)
		{
			preTop = (Node*)TopNode.u_Tag.Address;
			nNode->next = preTop;

			preTopNode.u_Tag.Address = (__int32)preTop;
			preTopNode.u_Tag.Tag = preTop->tag;

			newTopNode.u_Tag.Address = (__int32)nNode;
			newTopNode.u_Tag.Tag = nNode->tag;

			__int64 ret = InterlockedCompareExchange64((__int64*)&TopNode.TagAddress, newTopNode.TagAddress, preTopNode.TagAddress);
			
			if (ret == preTopNode.TagNode)
			{
				InterlockedIncrement(&Cnt);
				break;
			}
		}
	}

	T Pop()
	{
		Node* popNode;
		Node* nextTop;

		DoubleCAS popTopNode;
		DoubleCAS nextTopNode;

		while (1)
		{
			if (TopNode.TagAddrss == nullptr)
				break;

			popNode = (Node*)TopNode.u_Tag.Address;
			nextTop = popNode.next;

			popTopNode.u_Tag.Address = (__int32)popNode;
			popTopNode.u_Tag.Tag = popNode.tag;

			nextTopNode.u_Tag.Address = (__int32)nextTop;
			nextTopNode.u_Tag.Tag = nextTop.tag;

			__int64 ret = InterlockedCompareExchange64((__int64*)&TopNode.TagAddress, nextTopNode.TagAddress, popTopNode.TagAddress);
			if (ret == popTopNode.TagAddress)
			{
				T data = popNode->data;

				LockFreePool.Free(popNode);
				return data;
			}
		}
		return nullptr;
	}
};

#endif