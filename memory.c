#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "memory.h"

#define PAGE_SIZE 4096

static size_t* free_list[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int sizeRef[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

static void *alloc_from_ram(size_t size)
{
	assert((size % PAGE_SIZE) == 0 && "size must be multiples of 4096");
	void* base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	if (base == MAP_FAILED)
	{
		printf("Unable to allocate RAM space\n");
		exit(0);
	}
	return base;
}

static void free_ram(void *addr, size_t size)
{
	munmap(addr, size);
}

int getSize(size_t k){
	int ans = -1;
	for (int i = 0; i<=8; ++i){
		if (k<=sizeRef[i]){
			ans = i;
			break;
		}
	}
	if (ans!=-1){
		return ans;
	}
	return k;
}

size_t getSize_large(int k){
	size_t i = 1L;
	while (4096*i < k){
		i++;
	}
	return 4096*i;
}

/* 
void* pop_from_list(size_t* ptr){
	if (ptr!=NULL){
		size_t* metadata = ptr - ((size_t)ptr)/8;
		*(metadata+1) = *(metadata+1) -1;	// decrement available space
		*((size_t*)*(ptr+1)) = NULL;
		return (void*)ptr;
	}


} */

void* pop_from_list(int i){
	if (free_list[i]!=NULL){
		size_t* metadata = free_list[i] - ((size_t)free_list[i])/8;
		*(metadata+1) = *(metadata+1) -1;	// decrement available space check
		size_t * ptr = free_list[i];
		
		*((size_t*)*(free_list[i]+1)) = 0L;	// previous of next set to be null
		free_list[i] = (size_t*)*(free_list[i]+1);
		return (void*)ptr;
	}


}
void cleanup(int index, size_t* metaData){
	while (free_list[index] != NULL && (free_list[index] -((size_t )free_list[index] % 4096) == metaData)){
		pop_from_list(index);
	}
	
	if (free_list[index] != NULL){
		size_t* next =(size_t*) *(free_list[index]+1);
		size_t* ptr = free_list[index];
		while(next!= NULL){
			if (next - ((size_t)next %4096) == metaData){
				*(ptr+1) = *(next+1);
				*(size_t*)*(next+1) = (size_t)ptr;
			}
			ptr = next;
			next = (size_t*)*(next + 1);
			//prev = next;
		}
	}
	
	free_ram(metaData, 4096);
}

void add_to_list(size_t* ptr, size_t * block){
	*block = 0L;
	*(block+1) = (size_t)ptr;
	*ptr = (size_t)block;
	size_t* metaData = ptr - ((size_t)ptr%4096)/8;
	*(metaData+1) = *(metaData+1) + *(metaData);
	if (*(metaData+1)>=4096){
		int calcSize = getSize(*metaData);
		cleanup(calcSize, metaData);
	}
	ptr = block;
}


void myfree(void *ptr)
{
	size_t* page = (size_t*) ptr;
	page = page - ((size_t) page %4096);
	int size = *(page);
	if (size<=4080){
		int sz = getSize(size);
		size_t* list = free_list[sz];
		add_to_list(list, (size_t*) ptr);
	}
	else{
		free_ram((void*) page, size);
	}/* 
	printf("myfree is not implemented\n");
	abort(); */
}



void *mymalloc(size_t size)
{
	if (size > 4080){
		int calcSize = getSize(size);
		size_t * ptr = free_list[calcSize];
		if (ptr==NULL){
			size_t* page = (size_t *) alloc_from_ram(4096);
			size_t* endLimit = page + 4096;
			*page = size;
			page ++;
			*page = 4096 - 16;
			page ++;
			while(page < endLimit ){
				add_to_list(ptr, page);
				page += (size/8);
			}
		}
		return pop_from_list(calcSize);
	}
	
	else{
		int size = getSize_large(size+8);
		void* output = alloc_from_ram(size);
		size_t* output2 = (size_t *) output;
		*(output2) = size;
		output2 +=2;
		return (void*) output2;
	}

	/* printf("mymalloc is not implemented\n");
	abort();
	return NULL; */
}
