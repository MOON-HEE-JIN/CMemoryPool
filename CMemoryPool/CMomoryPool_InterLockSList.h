#pragma once
#include <Windows.h>
#include <malloc.h>
#include <vector>
#include <list>

template<typename T>
struct ListPool
{
	ListPool* next;
	T AllocValue;
};

template<typename T>
class CMemoryPoolManager
{
public:
	CMemoryPoolManager()
	{
		InitializeCriticalSection(&m_cspool);
	}
	~CMemoryPoolManager()
	{
		for (int i = 0; i < m_pool.size(); i++)
		{
			ListPool<T>* pDelete = nullptr;

			for (;;)
			{
				pDelete = m_pool[i];
				m_pool[i] = pDelete->next;
				if (pDelete == nullptr)
					break;

				delete pDelete;
			}
		}
		DeleteCriticalSection(&m_cspool);
	}
private:

	std::vector<ListPool<T>*> m_pool;

	CRITICAL_SECTION m_cspool;
	const int m_BlockCnt = 1000;

	ListPool<T>* NewAllocListPool()
	{
		ListPool<T>* pre = new ListPool<T>;

		ListPool<T>* ret = pre;
		for (int i = 1; i < m_BlockCnt; i++)
		{
			ListPool<T>* next = (ListPool<T>*)malloc(sizeof(ListPool<T>));

			pre->next = next;
			pre = next;
		}
		pre->next = nullptr;

		return ret;
	}
public:
	int GetAllocCnt() { return m_BlockCnt; }

	ListPool<T>* AllocListPool()
	{
		ListPool<T>* retpool = nullptr;
		EnterCriticalSection(&m_cspool);
		if (!m_pool.empty())
		{
			retpool = m_pool.back();
			m_pool.pop_back();
		}
		LeaveCriticalSection(&m_cspool);

		if (retpool == nullptr)
			retpool = NewAllocListPool();

		return retpool;
	}

	void FreeListPool(ListPool<T>* pool)
	{
		EnterCriticalSection(&m_cspool);
		m_pool.push_back(pool);
		LeaveCriticalSection(&m_cspool);
	}
};

template<typename T>
class MemoryPool
{
public:
	MemoryPool(CMemoryPoolManager<T>* owner, bool recycle = false)
	{
		m_pOwner = owner;
		m_freeCnt = 0;
		m_maxFreeCnt = m_pOwner->GetAllocCnt();

		m_bRecycle = recycle;
		m_RecycleCnt = m_maxFreeCnt / 5;
	}
	~MemoryPool()
	{
		ListPool<T>* pDelete;
		for (;;)
		{
			pDelete = m_pool;
			m_pool = pDelete->next;
			if (pDelete == nullptr)
				break;
			delete pDelete;
		}

		for (;;)
		{
			pDelete = m_freebegin;
			m_freebegin = pDelete->next;
			if (pDelete == nullptr)
				break;
			delete pDelete;
		}
	}
	CMemoryPoolManager<T>* m_pOwner;
private:
	static thread_local ListPool<T>* m_pool;
	static thread_local int m_freeCnt;
	static thread_local int m_maxFreeCnt;

	static thread_local bool m_bRecycle;
	static thread_local int m_RecycleCnt;

	static thread_local ListPool<T>* m_freebegin;
	static thread_local ListPool<T>* m_free;
public:
	T* Alloc()
	{
		if (m_pool == nullptr)
			m_pool = m_pOwner->AllocListPool();

		ListPool<T>* ret = m_pool;
		m_pool = ret->next;

		return &ret->AllocValue;
	}
	void Free(T* value)
	{
		ListPool<T>* pFree = reinterpret_cast<ListPool<T>*>(
			reinterpret_cast<char*>(value) - offsetof(ListPool<T>, AllocValue));

		m_freeCnt++;

		pFree->next = nullptr;

		if (m_freebegin == nullptr)
		{
			m_freebegin = pFree;
			m_free = pFree;
			return;
		}
		m_free->next = pFree;
		m_free = pFree;

		if (m_bRecycle)
		{
			if (m_freeCnt >= m_RecycleCnt)
			{
				if (m_pool != nullptr)
					m_pool->next = m_freebegin;
				else
					m_pool = m_freebegin;

				m_freebegin = nullptr;
				m_free = nullptr;
				m_freeCnt = 0;

				return;
			}
		}

		if (m_freeCnt < m_maxFreeCnt)
			return;

		m_pOwner->FreeListPool(m_freebegin);
		m_freebegin = nullptr;
		m_free = nullptr;
		m_freeCnt = 0;
	}
};

template<typename T>
thread_local ListPool<T>* MemoryPool<T>::m_pool = nullptr;
template<typename T>
thread_local int MemoryPool<T>::m_freeCnt = 0;
template<typename T>
thread_local int MemoryPool<T>::m_maxFreeCnt = 0;
template<typename T>
thread_local bool MemoryPool<T>::m_bRecycle = 0;
template<typename T>
thread_local int MemoryPool<T>::m_RecycleCnt = 0;
template<typename T>
thread_local ListPool<T>* MemoryPool<T>::m_freebegin = nullptr;
template<typename T>
thread_local ListPool<T>* MemoryPool<T>::m_free = nullptr;