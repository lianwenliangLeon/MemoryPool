#pragma once

#ifndef MemoryPool_H
#define MemoryPool_H

const size_t MINUMITSIZE = 64;
const size_t ADDR_ALIGN = 8;
const size_t SIZE_ALIGN = MINUMITSIZE;

class MemoryBlock
{
public:
	int backBlock;//后面还有多少连续的空余内存卡=块
	int ptrIndex;//指向这块内存块区域的指针的相对索引
	PtrMemoryBlock* ptr;//指向 指向这块内存块的指针
};

class PtrMemoryBlock
{
public:
	PtrMemoryBlock(PtrMemoryBlock * pool,size_t count);
	MemoryBlock* blockPtr;//指向连续内存块首的指针
	PtrMemoryBlock* pre;//前一个
	PtrMemoryBlock* next;//后一个
};

/*
内存池
*/
class MemoryPool 
{
private:
	void* memory;//所属可分配的内存区域
	size_t Size;
	MemoryBlock* ptrMemoryMap;//可分配内存地址
	PtrMemoryBlock* pfreeMemory;//现在用的链表
	PtrMemoryBlock* pfreeMemoryPool;//链表池 备用的
	size_t mem_used_size;//已分配的内存块
	size_t mem_map_pool_count; //记录链表池单元个数
	size_t free_mem_chunk_count;// 记录 现在使用的链表的结点数
	size_t mem_map_unit_count;
	size_t mem_block_count;// 记录内存块个数
public:
	static MemoryPool* CreateMemoryPool(void* pBuf, size_t PoolSize);
	void* GetMemory(size_t MemorySize);
	bool FreeMemory(void* memoryPtr);
	PtrMemoryBlock* front_pop();
	PtrMemoryBlock* create_list(size_t count);
	bool push_back(PtrMemoryBlock *& head,PtrMemoryBlock* elment);
	bool push_front(PtrMemoryBlock *& head,PtrMemoryBlock* element);
	bool deletePtr(PtrMemoryBlock* element);
	size_t addrToIndex(void* addr);
	size_t indexToAddr(size_t index);
	void* GetMemory(size_t getMemorySize);
	bool FreeMemory(void* ptrMemoryBlock);
};

#endif // !MemoryPool_H
