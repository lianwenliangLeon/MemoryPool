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
size_t check_align_block(size_t size)
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
	if (sizeof(MemoryPool) < sBufSize)
		return NULL;
	memset(pBuf, 0, sBufSize);
	MemoryPool* MP = (MemoryPool*)pBuf;
	size_t MemoryPoolSize = sizeof(MemoryPool);
	MP->mem_map_pool_count = (sBufSize - MemoryPoolSize + MINUMITSIZE - 1) / MINUMITSIZE;//向上取整 块数
	MP->mem_map_unit_count = (sBufSize - MemoryPoolSize + MINUMITSIZE - 1) / MINUMITSIZE;//向上取整 块数

	MP->ptrMemoryMap = (MemoryBlock*)((char*)pBuf + MemoryPoolSize);//可分配内存初始地址
	if (!MP->ptrMemoryMap)
		return NULL;
	
	MP->pfreeMemoryPool =(PtrMemoryBlock *)((char*)pBuf + MemoryPoolSize + sizeof(MemoryBlock) * MP->mem_map_unit_count);//指针起始地址
	if (!MP->pfreeMemoryPool)
		return NULL;

	MP->memory = (char*)pBuf + MemoryPoolSize + sizeof(MemoryBlock) * MP->mem_map_unit_count + sizeof(PtrMemoryBlock) * MP->mem_map_pool_count;//总的存储空间地址为 指针所占空间 加上内存块所占空间 加上内存池所占空间
	MP->Size = sBufSize - MemoryPoolSize - sizeof(MemoryBlock) * MP->mem_map_unit_count - sizeof(PtrMemoryBlock) * MP->mem_map_pool_count;
	size_t align = check_algin_addr(MP->memory);
	MP->Size = align;
	MP->Size = check_align_block(MP->Size);
	MP->mem_block_count = MP->Size / MINUMITSIZE;//块数

	//链表
	MP->pfreeMemoryPool = MP->create_list(MP->mem_map_pool_count);
	//初始化 pfree_mem_chunk 双向循环链表
	PtrMemoryBlock* tmp = MP -> front_pop();//取出一个指针 指向整个内存块
	tmp->pre = tmp;
	tmp->next = tmp;
	tmp->blockPtr = NULL;
	MP->mem_map_pool_count--;

	//初始化 此时内存块不被分割 相当于只有一块很大的
	MP->ptrMemoryMap[0].backBlock=MP->mem_block_count;                       
	MP->ptrMemoryMap[0].ptr = tmp;
	MP->ptrMemoryMap[MP->mem_block_count - 1].ptrIndex = 0;

	tmp->blockPtr = MP->ptrMemoryMap;
	MP->push_back(MP->pfreeMemory,tmp);
	MP->free_mem_chunk_count = 1;
	MP->mem_used_size = 0;
	
	return MP;
}

PtrMemoryBlock * MemoryPool::create_list(size_t count)//将链表区的单独PtrMemoryBlock链接为双向循环链表 并返回表头
{
	PtrMemoryBlock* pool = this->pfreeMemoryPool;
	PtrMemoryBlock* head = NULL;
	for (size_t i = 0; i < count; i++)
	{
		pool->pre = NULL;
		pool->next = head;
		if (head != NULL)
		{
			head->pre = pool;
		}
		head = pool;
		pool++;
	}
	return head;
}

void* MemoryPool::GetMemory(size_t sMemorySize)
{
	sMemorySize = check_align_size(sMemorySize);//对齐内存
	size_t index = 0;
	PtrMemoryBlock* tmp = this->pfreeMemory;
	for (index = 0; index < this->free_mem_chunk_count; index++)
	{
		if (tmp->blockPtr->backBlock * MINUMITSIZE >= sMemorySize)
			break;// 找足够大的区域
	}
}

PtrMemoryBlock* MemoryPool::front_pop()//首部元素出链表，并返回
{
	PtrMemoryBlock* pool = this->pfreeMemoryPool;

	if (!this->pfreeMemoryPool)
	{
		return NULL;
	}
	PtrMemoryBlock* tmp = pool;
	pool = tmp->next;
	pool->pre = NULL;
	return tmp;
}

bool MemoryPool::push_back(PtrMemoryBlock*& head, PtrMemoryBlock* element)//在链表尾部插入元素
{
	if (head == NULL)
	{
		head = element;
		head->pre = element;
		head->next = element;
		return true;
	}
	head->pre->next = element;
	element->pre = head->pre;
	head->pre = element;
	element->next = head;
	return true;
}

bool MemoryPool::push_front(PtrMemoryBlock*& head, PtrMemoryBlock* element)//在首部插入元素
{
	element->pre = NULL;
	element->next = head;
	if (head != NULL)
	{
		head->pre = element;
	}
	head = element;
	return true;
}