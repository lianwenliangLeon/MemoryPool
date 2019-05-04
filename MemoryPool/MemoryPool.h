#pragma once

#ifndef MemoryPool_H
#define MemoryPool_H

const size_t MINUMITSIZE = 64;
const size_t ADDR_ALIGN = 8;
const size_t SIZE_ALIGN = MINUMITSIZE;

class MemoryBlock
{
private:
	int backBlock;//后面还有多少连续的空余内存卡=块
	int ptrIndex;//指向这块内存块区域的指针的相对索引
	PtrMemoryBlock* ptr;//指向 指向这块内存块的指针
public:
	inline int GetBackBlock(){
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
	MemoryBlock* blockPtr;//指向连续内存块首的指针
	PtrMemoryBlock* pre;//前一个
	PtrMemoryBlock* next;//后一个
public:
	inline PtrMemoryBlock* getNextPtr() { return next; }
	inline PtrMemoryBlock* getPrePtr() { return pre; }
	inline MemoryBlock* getMemBlo() { return blockPtr; }
	PtrMemoryBlock(PtrMemoryBlock * pool,size_t count);
};

/*
内存池
*/
class MemoryPool 
{
private:
	void* memory;//所属可分配的内存区域
	size_t memorySize;
	MemoryBlock* ptrMemoryMap;
	PtrMemoryBlock* pfreeMemory;
	PtrMemoryBlock* pfreeMemoryPool;
	size_t mem_used_size;//已分配的内存块
	size_t mem_map_pool_count; //记录链表单元缓冲池中剩余的单元的个数，个数为0时不能分配单元给pfree_mem_chunk
	size_t free_mem_chunk_count;// 记录 pfree_mem_chunk链表中的单元个数 
	size_t mem_map_unit_count;
	size_t mem_block_count;// 一个 mem_unit 大小为 MINUNITSIZE
public:
	static MemoryPool* CreateMemoryPool(void* pBuf, size_t PoolSize);
	void* GetMemory(size_t MemorySize);
	bool FreeMemory(void* memoryPtr);
	PtrMemoryBlock* front_pop();
	PtrMemoryBlock* create_list(size_t count);
	bool push_back(PtrMemoryBlock* elment);
	bool push_front(PtrMemoryBlock* element);
	bool deletePtr(PtrMemoryBlock* element);
	size_t addrToIndex(void* addr);
	size_t indexToAddr(size_t index);
	void* GetMemory(size_t getMemorySize);
	bool FreeMemory(void* ptrMemoryBlock);
};

#endif // !MemoryPool_H
