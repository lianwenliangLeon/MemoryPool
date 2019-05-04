#pragma once

#ifndef MemoryPool_H
#define MemoryPool_H

const size_t MINUMITSIZE = 64;
const size_t ADDR_ALIGN = 8;
const size_t SIZE_ALIGN = MINUMITSIZE;

class MemoryBlock
{
private:
	int backBlock;//���滹�ж��������Ŀ����ڴ濨=��
	int ptrIndex;//ָ������ڴ�������ָ����������
	PtrMemoryBlock* ptr;//ָ�� ָ������ڴ���ָ��
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
	MemoryBlock* blockPtr;//ָ�������ڴ���׵�ָ��
	PtrMemoryBlock* pre;//ǰһ��
	PtrMemoryBlock* next;//��һ��
public:
	inline PtrMemoryBlock* getNextPtr() { return next; }
	inline PtrMemoryBlock* getPrePtr() { return pre; }
	inline MemoryBlock* getMemBlo() { return blockPtr; }
	PtrMemoryBlock(PtrMemoryBlock * pool,size_t count);
};

/*
�ڴ��
*/
class MemoryPool 
{
private:
	void* memory;//�����ɷ�����ڴ�����
	size_t memorySize;
	MemoryBlock* ptrMemoryMap;
	PtrMemoryBlock* pfreeMemory;
	PtrMemoryBlock* pfreeMemoryPool;
	size_t mem_used_size;//�ѷ�����ڴ��
	size_t mem_map_pool_count; //��¼����Ԫ�������ʣ��ĵ�Ԫ�ĸ���������Ϊ0ʱ���ܷ��䵥Ԫ��pfree_mem_chunk
	size_t free_mem_chunk_count;// ��¼ pfree_mem_chunk�����еĵ�Ԫ���� 
	size_t mem_map_unit_count;
	size_t mem_block_count;// һ�� mem_unit ��СΪ MINUNITSIZE
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
