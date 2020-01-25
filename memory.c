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
static int sizeRef[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4080};

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

size_t getSize(size_t k){
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
		size_t* metadata = free_list[i] - ((size_t)free_list[i] %4096)/8;
		*(metadata+1) = *(metadata+1) - *(metadata);	// decrement available space check
		size_t * ptr = free_list[i];	// this has to be removed from linklist
		if (*(ptr+1) != 0L){
			*((size_t*)*(ptr+1)) = 0L;	// previous of next set to be null
		}
		
		free_list[i] = (size_t*)*(ptr+1);
		return (void*)ptr;
	}
	else{
		printf("%d\n", 2018082);	//just for debugging :)
	}


}
void cleanup(int index, size_t* metaData){
	while (free_list[index] != NULL && (free_list[index] -((size_t )free_list[index] % 4096)/8 == metaData)){
		pop_from_list(index);	//remove blocks from top corresoponding to same page
	}
	
	if (free_list[index] != NULL){
		size_t* next =(size_t*) *(free_list[index]+1);
		size_t* ptr = free_list[index];
		while(next!= NULL){
			if (next - ((size_t)next %4096)/8 == metaData){
				*(ptr+1) = *(next+1); //ptr.next <- next.next
				*(size_t*)*(next+1) = (size_t)ptr;	//next.next.prev <- ptr
			}
			else{
				ptr = next;
			}
			 
			next = (size_t*)*(next + 1);	//next<- next.next
			
		}
	}
	
	free_ram(metaData, 4096);
}

void add_to_list(int sz, size_t* block){
	printf("Block : %ld\n", (size_t)block);
	size_t* ptr = free_list[sz];
	*block = 0L;	
	if (ptr != NULL){
		*(block+1) = (size_t)ptr;
		*ptr = (size_t)block;
	}
	else{
		*(block+1) = 0L;
	}
	//previous pointer
		//next pointer
		//previous to existing linklist
	size_t* metaData = block - ((size_t)block%4096)/8;
	*(metaData+1) = *(metaData+1) + *(metaData);	// available <- available + size
	free_list[sz] = block;	// updating free_list
	if (*(metaData+1)>=4096){
		int calcSize = getSize(*metaData);
		cleanup(calcSize, metaData);	//cleanup freelist at index calcSize
	}
	//*ptr = block;
}


void myfree(void *ptr)
{
	size_t* page = (size_t*) ptr;
	page = page - ((size_t) page %4096)/8;
	size_t size = *(page);
	if (size<=4080){
		int sz = getSize(size);
		add_to_list(sz, (size_t*) ptr);
	}
	else{
		free_ram((void*) page, size);
	}/* 
	printf("myfree is not implemented\n");
	abort(); */
}



void *mymalloc(size_t size)
{
	//printf("Size : %ld\n", size);
	if (size < 4080){
		int sz = getSize(size);
		size_t calcSize = sizeRef[sz];
		size_t * ptr = free_list[sz];
		 
		if (ptr==NULL){
			size_t* page = (size_t *) alloc_from_ram(4096);
			//printf("start Page : %ld\n", (size_t)page);
			//printf("CalcSize : %ld\n", (size_t)calcSize);
			
			size_t* endLimit = page + (4096 - calcSize)/8;
			//endLimit = endLimit - (calcSize/8);
			//printf("Endlimit : %ld\n", (size_t)endLimit);
			*page = calcSize;
			page ++;
			*page = 16 ;
			page ++;
			int counter = 0;	//hata dena baadme
			ptr = page;
			counter++;
			*ptr = 0L;
			*(ptr+1) = 0L;			
			//free_list[sz] = ptr;
			page += (calcSize/8);
			//printf("Page : %ld\n", (size_t)page);
			while(page <= endLimit ){
				//printf("counter:%d\n", counter);
				add_to_list(sz, page);
				page += (calcSize/8);
				//printf("Page : %ld\n", (size_t)page);
				counter++;
			}
			//free_list[getSize(size)] = ptr;
			return (void*)ptr;
		}
		else{
			return pop_from_list(sz);
		}
		
	}
	
	else{
		int size = getSize_large(size+16);
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
