#pragma once

#ifndef MemoryPool_H
#define MemoryPool_H

const size_t MINUMITSIZE = 64;
const size_t ADDR_ALIGN = 8;
const size_t SIZE_ALIGN = MINUMITSIZE;

class MemoryBlock
{
public:
	int backBlock;//���滹�ж��������Ŀ����ڴ濨=��
	int ptrIndex;//ָ������ڴ�������ָ����������
	PtrMemoryBlock* ptr;//ָ�� ָ������ڴ���ָ��
};

class PtrMemoryBlock
{
public:
	PtrMemoryBlock(PtrMemoryBlock * pool,size_t count);
	MemoryBlock* blockPtr;//ָ�������ڴ���׵�ָ��
	PtrMemoryBlock* pre;//ǰһ��
	PtrMemoryBlock* next;//��һ��
};

/*
�ڴ��
*/
class MemoryPool 
{
private:
	void* memory;//�����ɷ�����ڴ�����
	size_t Size;
	MemoryBlock* ptrMemoryMap;//�ɷ����ڴ��ַ
	PtrMemoryBlock* pfreeMemory;//�����õ�����
	PtrMemoryBlock* pfreeMemoryPool;//����� ���õ�
	size_t mem_used_size;//�ѷ�����ڴ��
	size_t mem_map_pool_count; //��¼����ص�Ԫ����
	size_t free_mem_chunk_count;// ��¼ ����ʹ�õ�����Ľ����
	size_t mem_map_unit_count;
	size_t mem_block_count;// ��¼�ڴ�����
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
