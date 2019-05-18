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
	MP->ptrMemoryMap[MP->mem_block_count - 1].strIndex = 0;

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
		tmp = tmp->next;
	}
	if (index == free_mem_chunk_count)
	{
		return NULL;
	}
	mem_used_size += sMemorySize;//已用空间增加
	if (tmp->blockPtr->backBlock == sMemorySize)
	{
		//需要的内存刚好和当前链表指向的内存块大小相等
		size_t current_index = (tmp->blockPtr - ptrMemoryMap);//计算当前块头的相对索引
		deletePtr(tmp);/*将节点从链表中分离*/
		tmp->blockPtr->ptr = NULL;
		push_front(pfreeMemoryPool, tmp);//放回链表头备用池

		free_mem_chunk_count--;//已经使用的少了一个
		mem_map_pool_count++;//池里多了一个
		return indexToAddr(current_index);
	}
	else
	{
		//如果当前指针所指的空间大于用户所申请的空间
		MemoryBlock copy;
		copy.backBlock = tmp->blockPtr->backBlock;
		copy.ptr = tmp;


		MemoryBlock* current_block = tmp->blockPtr;
		current_block->backBlock = sMemorySize / MINUMITSIZE;//当前所需要的
		size_t current_index = (current_block - ptrMemoryMap);
		ptrMemoryMap[current_index + current_block->backBlock - 1].strIndex = current_index;
		current_block->ptr = NULL;//表示当前内存块已被分配
		ptrMemoryMap[current_index + current_block->backBlock].backBlock = copy.backBlock - current_block->backBlock;
		ptrMemoryMap[current_index + current_block->backBlock].ptr = copy.ptr;//指针还是原来那个

		tmp->blockPtr = &(ptrMemoryMap[current_index + current_block->backBlock]);

		size_t end_index = current_index + copy.backBlock - 1;
		ptrMemoryMap[end_index].strIndex = current_index + current_block->backBlock;
		return indexToAddr(current_index);
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

bool MemoryPool::deletePtr(PtrMemoryBlock * element)
{
	if (element == NULL)
	{
		return;
	}
	else if (element == pfreeMemory)//element在头部
	{
		if (pfreeMemory->pre == pfreeMemory)
		{
			pfreeMemory = NULL;
		}
		else
		{
			pfreeMemory = element->next;
			pfreeMemory->pre = element->pre;
			pfreeMemory->pre->next = pfreeMemory;
		}
	}
	//element在尾部
	else if (element->next == pfreeMemory)
	{
		pfreeMemory->pre = element->pre;
		element->pre->next = pfreeMemory;
	}
	else
	{
		element->pre->next = element->next;
		element->next->pre = element->pre;
	}
	element->pre = NULL;
	element->next = NULL;
	
	return true;
}

void* MemoryPool::indexToAddr(size_t index)
{
	char* p = (char*)memory;
	void* ret = (void*)(p + index * MINUMITSIZE);//找到位置
	return ret;
}

bool MemoryPool::FreeMemory(void * memoryPtr)
{
	size_t current_index = addrToIndex(memoryPtr);
	size_t size = ptrMemoryMap[current_index].backBlock * MINUMITSIZE;
	//内存碎片优化 判断相邻内存块是否可以合并
	MemoryBlock* pre_block = NULL;
	MemoryBlock* next_block = NULL;
	MemoryBlock* current_block = &(ptrMemoryMap[current_index]);
	//
	if (current_index == 0)//首部的内存块
	{
		if (current_block->backBlock < mem_block_count)//后面还有
		{
			next_block = &(ptrMemoryMap[current_index + current_block->backBlock]);//之后的一块没有被使用
			if (next_block->ptr != NULL)
			{
				next_block->ptr->blockPtr = current_block;
				ptrMemoryMap[current_index + current_block->backBlock + next_block->backBlock - 1].strIndex = current_index;
				current_block->backBlock += next_block->backBlock;
				current_block->ptr = next_block->ptr;
				next_block->ptr = NULL;
			}
			else//之后的一块已经被使用
			{
				PtrMemoryBlock* new_ptr = front_pop();
				new_ptr->blockPtr = current_block;
				current_block->ptr = new_ptr;
				push_back(pfreeMemory, new_ptr);//从备用链表池里取出一个。
				mem_map_pool_count--;
				free_mem_chunk_count++;
			}
		}
	}
	else if (current_index == mem_block_count - 1)//如果是最后一个
	{
		if (current_block->backBlock < mem_block_count)//后面还有？
		{
			pre_block = &(ptrMemoryMap[current_index - 1]);//前一个内存块
			size_t index = pre_block->strIndex;//index等于之后还有多少内存块 讲真我看不懂
			pre_block = &(ptrMemoryMap[index]);//这个等于前面的空缺？ 我觉得这里有错
			if (pre_block->ptr != NULL)//如果前面的内存块没有被分配出去
			{
				ptrMemoryMap[current_index + current_block->backBlock - 1].strIndex = current_index - pre_block->backBlock;
				pre_block->backBlock += current_block->backBlock;
				current_block->ptr = NULL;
			}
			else
			{
				PtrMemoryBlock* new_ptr = front_pop();
				new_ptr->blockPtr = current_block;
				current_block->ptr = new_ptr;
				push_back(pfreeMemory, new_ptr);
				mem_map_pool_count--;
				free_mem_chunk_count++;
			}
		}
		else
		{
			PtrMemoryBlock* new_ptr = front_pop();
			new_ptr->blockPtr = current_block;
			current_block->ptr = new_ptr;
			push_back(pfreeMemory, new_ptr);
			mem_map_pool_count--;
			free_mem_chunk_count++;
		}
	}
	else
	{
		next_block = &(ptrMemoryMap[current_index + current_block->backBlock]);
		pre_block = &(ptrMemoryMap[current_index - 1]);
		size_t index = pre_block->strIndex;
		pre_block = &(ptrMemoryMap[index]);
		bool is_back_merge = false;
		if (next_block->ptr == NULL && pre_block->ptr == NULL);//前后都已经被分配
		{
			PtrMemoryBlock* new_ptr = front_pop();
			new_ptr->blockPtr = current_block;
			current_block->ptr = new_ptr;
			push_back(pfreeMemory, new_ptr);
			mem_map_pool_count--;
			free_mem_chunk_count++;
		}
		if (next_block->ptr != NULL)
		{
			next_block->ptr->blockPtr = current_block;
			ptrMemoryMap[current_index + current_block->backBlock + next_block->backBlock - 1].strIndex = current_index;
			current_block->backBlock += next_block->backBlock;
			current_block->ptr = next_block->ptr;
			next_block->ptr = NULL;
			is_back_merge = true;
		}
		if (pre_block->ptr != NULL)
		{
			ptrMemoryMap[current_index + current_block->backBlock - 1].strIndex = current_index - pre_block->backBlock;
			pre_block->backBlock += current_block->backBlock;
			if (is_back_merge)
			{
				deletePtr(current_block->ptr);
				push_front(pfreeMemory, current_block->ptr);
				free_mem_chunk_count--;
				mem_map_pool_count++;
			}
			current_block->ptr = NULL;
		}
	}
	mem_used_size -= size;
}

size_t MemoryPool::addrToIndex(void* addr)
{
	char* start = (char*)(memory);
	char* p = (char*)addr;
	size_t index = (p - start) / MINUMITSIZE;
	return index;
}