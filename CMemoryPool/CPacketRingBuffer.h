#pragma once
#include <winnt.h>

#define BUFFERSIZE 500

class CPacketRingBuffer
{
public:
	CPacketRingBuffer();
	~CPacketRingBuffer();

	int Enqueue();
	int Dequeue();

private:
	PVOID buffer[BUFFERSIZE];

	int ReadIndex;	// 읽어올 Packet 주소
	int WriteIndex; // 쓰기할 Packet 주소

};