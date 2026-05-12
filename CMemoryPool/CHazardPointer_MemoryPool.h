#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

template<typename T>
class CHMemoryPool
{
	struct Node {
		T data;
		Node* next;
	};

	std::atomic<Node*> m_freeList;
	
	std::atomic<int> m_debug_Alloc = 0;
	std::atomic<int> m_debug_Free = 0;

	//  동시에 사용할수 있는 Thread 개수
	static const int MAX_COUNT = 8;
	struct HazardRecord 
	{
		std::atomic<std::thread::id> m_ThreadID;
		std::atomic<Node*> m_pProtectedPointer;
		HazardRecord() 
		{
			m_ThreadID = std::thread::id();
			m_pProtectedPointer = nullptr;
		}
	};
	static HazardRecord hazardRecs[MAX_COUNT];

	// thread_ID 당 보호 할 pointer 주소
	struct HazardOwner 
	{
		HazardRecord* rec;
		std::atomic<Node*>& GetProtectedPointer() { return rec->m_pProtectedPointer; }
		HazardOwner()
		{
			rec = nullptr;

			for (int i = 0; i < MAX_COUNT; i++)
			{
				// 초기화 값으로 0 ( 아무스레드도 할당되지않음 )
				std::thread::id preID;
				// atomic 연산 threadID 와 preID 가 같다면 지금 threadid 로 교체
				if (hazardRecs[i].m_ThreadID.compare_exchange_strong(preID, std::this_thread::get_id()))
				{
					// (스레드 할당이 되지 않을시 rec 에 관리 rec 를 등록)
					rec = &hazardRecs[i];
					break;
				}
			}
		}
		~HazardOwner()
		{
			rec->m_ThreadID.store(std::thread::id());
			rec->m_pProtectedPointer.store(nullptr);
		}
	};

	// TLS 사용 하여 스레드 마다 하자드 포인트 생성
	static thread_local HazardOwner TLS_hazOwner;

	std::atomic<Node*>& GetHazardPool()
	{
		return TLS_hazOwner.GetProtectedPointer();
	}

	// 반환된 포인터 한계점
	static const int RETIRE_THRESHOLD = 500;
	static thread_local std::vector<Node*> m_retireList;

	void relcaimRetired()
	{
		std::vector<Node*> vec_hazard;
		for (int i = 0; i < MAX_COUNT; i++)
		{
			Node* pNode = hazardRecs[i].m_pProtectedPointer.load();
			// 보호 중인 pointer 를 검색하여 추가
			if (pNode != nullptr)
				vec_hazard.push_back(pNode);
		}

		auto iter = m_retireList.begin();
		for (iter; iter != m_retireList.end();)
		{
			Node* pNode = *iter;
			bool bUse = false;
			for (Node* hp : vec_hazard)
			{
				if (pNode == hp)
				{
					bUse = true;
					break;
				}
				// else 는 누군가 alloc 할려고 보호하는 주소
				// 이를 무시하고 해제한다면 aba 문제 발생
			}

			if (!bUse)
			{
				// 보호 받지 않은 주소만 지우기
				iter = m_retireList.erase(iter);

				Node* oldHead = m_freeList.load();
				do {
					pNode->next = oldHead;
				} while (!m_freeList.compare_exchange_weak(oldHead, pNode));
				m_debug_Free.fetch_add(1);
			}
			else
			{
				iter++;
			}
		}
	}

public:
	CHMemoryPool() { m_freeList = nullptr; };
	int GetAllocCount() { return m_debug_Alloc; }
	int GetFreeCount() { return m_debug_Free; }
private:
	Node* allocateNode()
	{
		while (1)
		{
			Node* oldHead = m_freeList.load();
			// 보호 주소 oldHead if oldHead 가 free 될려고 한다면
			// 보호 주소 이기 때문에 살리기 하지만 free 된다는 건 누가
			// 사용 이 끝났다는 뜻 alloc 에서 이미 누가 가져갔다는 것을 의미
			// 루프를 다시돌면서 보호주소를 다시 할당할 예정 이과정에서 ABA 해결

			GetHazardPool().store(oldHead);
			if (oldHead != m_freeList.load()) 
				continue;
			if (!oldHead)
			{
				// 이미 할당된 주소 보호 해제
				GetHazardPool().store(nullptr);
				m_debug_Alloc.fetch_add(1);
				return new Node();
			}
			Node* next = oldHead->next;
			if (m_freeList.compare_exchange_strong(oldHead, next))
			{
				// 이미 할당된 주소 보호 해제
				GetHazardPool().store(nullptr);
				oldHead->next = nullptr;
				return oldHead;
			}
		}
	}

	void releaseNode(Node* node)
	{
		node->next = nullptr;
		m_retireList.push_back(node);

		if (m_retireList.size() >= RETIRE_THRESHOLD)
			relcaimRetired();
	}

public:
	T* Alloc()
	{
		Node* node = allocateNode();
		return &node->data;
	}

	void Free(T* data)
	{
		releaseNode(reinterpret_cast<Node*>(reinterpret_cast<char*>(data) - offsetof(Node, data)));
	}
};

template<typename T>
typename CHMemoryPool<T>::HazardRecord CHMemoryPool<T>::hazardRecs[CHMemoryPool<T>::MAX_COUNT];

template<typename T>
thread_local typename CHMemoryPool<T>::HazardOwner CHMemoryPool<T>::TLS_hazOwner;

template<typename T>
thread_local std::vector<typename CHMemoryPool<T>::Node*> CHMemoryPool<T>::m_retireList;
