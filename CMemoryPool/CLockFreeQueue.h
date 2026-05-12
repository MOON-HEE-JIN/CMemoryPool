#pragma once
#pragma once
#include <Windows.h>
#include "CMemoryPool.h"
#include "CHazardPointer_MemoryPool.h"
#include <iostream>

//
// LockFree 를 이용한 Queue 
// 내부에서 LockFreeMemoryPool 를 사용중 
// 내부의 LockFreeMemoryPool 은 Queue 를 위한 MemoryPool
//

#ifdef WIN64

template<typename T>
class CLockFreeQueue
{
private:
    struct Node
    {
        T data;
        __int32 tag;
        unsigned int e_refCnt;
        unsigned int d_refCnt;
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

    DoubleCAS Head = { 0 };
    DoubleCAS Tail = { 0 };
    unsigned int Size;

    __int16 Unique = 0;

    CHMemoryPool<Node> LockFreePool;
public:
    int GetMemoryPoolSize() { return LockFreePool.GetAllocCount(); }
    int GetSize() { return Size; }
    CLockFreeQueue()
    {
        Node* Dummy = LockFreePool.Alloc();
        Dummy->next = nullptr;
        Dummy->tag = 0;
        Size = 0;

        Head.u_Tag.Address = (__int64)Dummy;
        Head.u_Tag.Tag = 0;
        Tail.u_Tag.Address = (__int64)Dummy;
        Tail.u_Tag.Tag = 0;

        Unique = 0;
    }

    ~CLockFreeQueue()
    {
        Node* pNode = (Node*)Head.u_Tag.Address;
        Node* dNode;
        for (;;)
        {
            if (pNode == nullptr)
                break;
            dNode = pNode;
            pNode = dNode->next;

            LockFreePool.Free(dNode);
        }
    }

    void Enqueue(T t)
    {
        unsigned __int16 value = _InterlockedIncrement16(&Unique);

        Node* new_DataNode = LockFreePool.Alloc();

        new_DataNode->data = t;
        new_DataNode->next = nullptr;
        new_DataNode->tag = value;
        new_DataNode->e_refCnt = 0;
        new_DataNode->d_refCnt = 0;
        Node* preNode;
        Node* nextNode;

        DoubleCAS curTail;
        DoubleCAS nextTail;

        while (1)
        {
            preNode = (Node*)Tail.u_Tag.Address;
            nextNode = preNode->next;
            //((Node*)(Tail.u_Tag.Address))->next;
            if (nextNode == nullptr)
            {
                // 지금 preNode->next == nullptr 이 라면 Enqueue 를 한다 (비었있으니까 넣고본다)
                if (InterlockedCompareExchangePointer((PVOID*)&((Node*)(Tail.u_Tag.Address))->next, new_DataNode, nullptr) == nullptr)
                {
                    // Enqueue 를 성공하였고 Tail 의 값을 이동시켜준다
                    // 여기서라면 Tail->next == new_DataNode 인 상태, Tail 은 아직 바뀌지 않은 상태
                    curTail.u_Tag.Address = (__int64)preNode;
                    curTail.u_Tag.Tag = preNode->tag;

                    nextTail.u_Tag.Address = (__int64)new_DataNode;
                    nextTail.u_Tag.Tag = new_DataNode->tag;

                    // 실질적으로 Tail 을 이동시키는 구간
                    __int64 ret = InterlockedCompareExchange64((__int64*)&Tail.TagAddress, (__int64)nextTail.TagAddress, (__int64)curTail.TagAddress);

                    InterlockedIncrement(&new_DataNode->e_refCnt);
                    if (new_DataNode->e_refCnt > 1)
                    {
                        std::cout << "Enqueue 하는 과정 중 중복 삽입\n";
                    }

                    if (ret != (__int64)curTail.TagAddress)
                    {
#ifdef _DEBUG
                        // Tail 의 이동이 실패 하였을때
                        // 하지만 Enqueue 를 성공하였으므로 성공 사례
                        // Head 가 Tail 를 앞지를수도 있기때문에
                        std::cout << "Tail Enqueue 과정중 Tail 의 이동 실행 다른 Thread 에서 Tail 을 이동\n";
#endif // _DEBUG
                    }
                    break;
                }

            }
        }
        InterlockedIncrement(&Size);
    }

    T Dequeue()
    {
        DoubleCAS curHead;
        DoubleCAS nextHead;

        Node* curNode;
        Node* nextNode;
        while (1)
        {
            // Head 는 DummyNode 를 가르키고 있기 때문에 nullptr 이 될수없다
            // 실제 나가야할 데이터는 DummyNode->next 즉 nextNode 이다
            curNode = (Node*)Head.u_Tag.Address;
            nextNode = curNode->next;

            // 실제 나가야할 데이터가 없다면 return nullptr;
            if (nextNode == nullptr)
                return nullptr;

            curHead.u_Tag.Address = (__int64)curNode;
            curHead.u_Tag.Tag = curNode->tag;

            nextHead.u_Tag.Address = (__int64)nextNode;
            nextHead.u_Tag.Tag = nextNode->tag;

            // 뽑을수 있는 데이터가 있는 상황 nextNode != nullptr
            if (InterlockedCompareExchange64((__int64*)&Tail.TagAddress, (__int64)nextHead.TagAddress, (__int64)curHead.TagAddress) == curHead.TagAddress)
            {
#ifdef _DEBUG
                {
                    // Tail, Head 가 같다면 Head 가 이동하면서 Tail 을 앞지르게 된다
                    // Tail 의 이동
                    std::cout << "Head 가 Tail 보다 앞서 나갈 가능성\n";
#endif // DEBUG
                }

                // Head 의 이동, 실질적 데이터 뽑기
                if (InterlockedCompareExchange64((__int64*)&Head.TagAddress, (__int64)nextHead.TagAddress, (__int64)curHead.TagAddress) == curHead.TagAddress)
                {

                    InterlockedIncrement(&nextNode->d_refCnt);
                    if (nextNode->d_refCnt > 1)
                    {
                        std::cout << "Dequeue 하는 과정 중 중복 추출\n";
                    }

                    T retData = nextNode->data;

                    LockFreePool.Free(curNode);
                    InterlockedDecrement(&Size);

                    return retData;
                }

            }
            InterlockedDecrement(&Size);
        }

    };
};

#elif WIN32


template<typename T>
class CLockFreeQueue
{
private:
    struct Node
    {
        T data;
        __int32 tag;
        unsigned int e_refCnt;
        unsigned int d_refCnt;
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

    DoubleCAS Head = { 0 };
    DoubleCAS Tail = { 0 };
    unsigned int Size;

    unsigned int Unique = 0;

    CMemoryPool<Node> LockFreePool;
public:
    int GetMemoryPoolSize() { return LockFreePool.GetAllocCount(); }
    int GetSize() { return Size; }
    CLockFreeQueue()
    {
        Node* Dummy = LockFreePool.Alloc();
        Dummy->next = nullptr;
        Dummy->tag = 0;
        Size = 0;

        Head.u_Tag.Address = (__int32)Dummy;
        Head.u_Tag.Tag = 0;
        Tail.u_Tag.Address = (__int32)Dummy;
        Tail.u_Tag.Tag = 0;
        
        Unique = 0;
    }

    ~CLockFreeQueue()
    {
        Node* pNode = (Node*)Head.u_Tag.Address;
        Node* dNode;
        for (;;)
        {
            if (pNode == nullptr)
                break;
            dNode = pNode;
            pNode = dNode->next;

            LockFreePool.Free(dNode);
        }
    }

    void Enqueue(T t)
    {
        unsigned int value = _InterlockedIncrement(&Unique);

        Node* new_DataNode = LockFreePool.Alloc();

        new_DataNode->data = t;
        new_DataNode->next = nullptr;
        new_DataNode->tag = value;
        new_DataNode->e_refCnt = 0;
        new_DataNode->d_refCnt = 0;
        Node* preNode;
        Node* nextNode;

        DoubleCAS curTail;
        DoubleCAS nextTail;

        while (1)
        {
            preNode = (Node*)Tail.u_Tag.Address;
            nextNode = preNode->next;
            //((Node*)(Tail.u_Tag.Address))->next;
            if (nextNode == nullptr)
            {
                // 지금 preNode->next == nullptr 이 라면 Enqueue 를 한다 (비었있으니까 넣고본다)
                if (InterlockedCompareExchangePointer((PVOID*)&((Node*)(Tail.u_Tag.Address))->next, new_DataNode, nullptr) == nullptr)
                {
                    // Enqueue 를 성공하였고 Tail 의 값을 이동시켜준다
                    // 여기서라면 Tail->next == new_DataNode 인 상태, Tail 은 아직 바뀌지 않은 상태
                    curTail.u_Tag.Address = (__int32)preNode;
                    curTail.u_Tag.Tag = preNode->tag;

                    nextTail.u_Tag.Address = (__int32)new_DataNode;
                    nextTail.u_Tag.Tag = new_DataNode->tag;

                    // 실질적으로 Tail 을 이동시키는 구간
                    __int64 ret = InterlockedCompareExchange64((__int64*)&Tail.TagAddress, (__int64)nextTail.TagAddress, (__int64)curTail.TagAddress);
                    
                    InterlockedIncrement(&new_DataNode->e_refCnt);
                    if (new_DataNode->e_refCnt > 1)
                    {
                        std::cout << "Enqueue 하는 과정 중 중복 삽입\n";
                    }

                    if (ret != (__int64)curTail.TagAddress)
                    {
#ifdef _DEBUG
                        // Tail 의 이동이 실패 하였을때
                        // 하지만 Enqueue 를 성공하였으므로 성공 사례
                        // Head 가 Tail 를 앞지를수도 있기때문에
                        std::cout << "Tail Enqueue 과정중 Tail 의 이동 실행 다른 Thread 에서 Tail 을 이동\n";
#endif // _DEBUG
                    }
                    break;
                }

            }
        }
        InterlockedIncrement(&Size);
    }

    T Dequeue()
    {
        DoubleCAS curHead;
        DoubleCAS nextHead;

        Node* curNode;
        Node* nextNode;
        while (1)
        {
            // Head 는 DummyNode 를 가르키고 있기 때문에 nullptr 이 될수없다
            // 실제 나가야할 데이터는 DummyNode->next 즉 nextNode 이다
            curNode = (Node*)Head.u_Tag.Address;
            nextNode = curNode->next;

            // 실제 나가야할 데이터가 없다면 return nullptr;
            if (nextNode == nullptr)
                return nullptr;

            //더느림 13줄 더느림 > 5줄
            //curHead.TagAddress = ((__int64)curNode->tag << 32) | (__int64)curNode;

            curHead.u_Tag.Address = (__int32)curNode;
            curHead.u_Tag.Tag = curNode->tag;

            nextHead.u_Tag.Address = (__int32)nextNode;
            nextHead.u_Tag.Tag = nextNode->tag;

            // 뽑을수 있는 데이터가 있는 상황 nextNode != nullptr
            if (InterlockedCompareExchange64((__int64*)&Tail.TagAddress, (__int64)nextHead.TagAddress, (__int64)curHead.TagAddress) == curHead.TagAddress)
#ifdef _DEBUG
            {
                // Tail, Head 가 같다면 Head 가 이동하면서 Tail 을 앞지르게 된다
                // Tail 의 이동
                std::cout << "Head 가 Tail 보다 앞서 나갈 가능성\n";
#endif // DEBUG
            }

            // Head 의 이동, 실질적 데이터 뽑기
            if (InterlockedCompareExchange64((__int64*)&Head.TagAddress, (__int64)nextHead.TagAddress, (__int64)curHead.TagAddress) == curHead.TagAddress)
            {

                InterlockedIncrement(&nextNode->d_refCnt);
                if (nextNode->d_refCnt > 1)
                {
                    std::cout << "Dequeue 하는 과정 중 중복 추출\n";
                }

                T retData = nextNode->data;

                LockFreePool.Free(curNode);
                InterlockedDecrement(&Size);

                return retData;
            }

        }
        InterlockedDecrement(&Size);
    }
   
};


#endif // _WIN32
