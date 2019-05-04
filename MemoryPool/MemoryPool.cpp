#include"MemoryPool.h"
#include<memory>

//内存池起始地址对齐 向上取整
size_t check_algin_addr(void*& pBuf)
{
	size_t align = 0;
	size_t addr = (int)pBuf;
	align = (ADDR_ALIGN - addr % ADDR_ALIGN) % ADDR_ALIGN;
	pBuf = (char*)pBuf + align;
	return align;
}

/*内存块大小对齐*/
size_t check_algin_addr(size_t size)
{
	size_t align = size % MINUMITSIZE;

	return size - align;
}

/*分配内存大小对齐到SIZE_ALIGN*/
size_t check_align_size(size_t size)
{
	size = (size + SIZE_ALIGN - 1) / SIZE_ALIGN * SIZE_ALIGN;
	return size;
}

/*
构建内存池
pBuf:给定的内存起始地址
sBufSize：给定的内存大小
*/
MemoryPool * MemoryPool::CreateMemoryPool(void* pBuf, size_t sBufSize)
{
	memset(pBuf, 0, sBufSize);
	MemoryPool* MP = (MemoryPool*)pBuf;
	size_t MemoryPoolSize = sizeof(MemoryPool);
	MP->mem_map_pool_count = (sBufSize - MemoryPoolSize + MINUMITSIZE - 1) / MINUMITSIZE;//向上取整 块数
	MP->mem_map_unit_count = (sBufSize - MemoryPoolSize + MINUMITSIZE - 1) / MINUMITSIZE;//向上取整 块数
	MP->ptrMemoryMap = (MemoryBlock*)((char*)pBuf + MemoryPoolSize);
}