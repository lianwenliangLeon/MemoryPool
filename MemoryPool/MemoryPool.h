#pragma once

#ifndef MemoryPool_H
#define MemoryPool_H

const int blockSize = 64;

class MemoryBlock
{
private:
	int backBlock;
	int ptrIndex;
	PtrMemoryBlock* ptr;
public:
	inline int GetBackBlock()
	{
		return backBlock;
	}
	inline int GetPtrIndex()
	{
		return ptrIndex;
	}
	inline PtrMemoryBlock* GetPtr()
	{
		return ptr;
	}
};

class PtrMemoryBlock
{
private:
	MemoryBlock* blockPtr;
	PtrMemoryBlock* pre;
	PtrMemoryBlock* next;
public:
	inline PtrMemoryBlock* getNextPtr() { return next; }
	inline PtrMemoryBlock* getPrePtr() { return pre; }
	inline MemoryBlock* getMemBlo() { return blockPtr; }
};

class MemoryPool 
{
	PtrMemoryBlock* head;
	int freeMemoryBlock;
	void* memory;
};

#endif // !MemoryPool_H
