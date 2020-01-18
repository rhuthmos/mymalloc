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
#include <math.h> 
#define PAGE_SIZE 4096

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

int getSize(int k){
	int ans = -1;
	for (int i = 4; i<=12; ++i){
		if (k<pow(2,i)){
			ans = i-4;
			break;
		}
	}
	if (ans!=-1){
		return ans;
	}
	return k;
}

void myfree(void *ptr)
{
	printf("myfree is not implemented\n");
	abort();
}

void add_to_list(long long int* ptr, long long int * block){

}

long long int* pop_from_list(long long int* ptr){

}

void *mymalloc(size_t size)
{
	int calcSize = getSize(size);

	if (calcSize<9){
		long long int * ptr = free_list[calcSize];
		int size = pow(2, calcSize+4);
		if (ptr==NULL){
			long long int* page = (long long int *) alloc_from_ram(4096);
			long long int* endLimit = page + 4096;
			*page = size;
			page ++;
			*page = 4096 - 16;
			page ++;
			while(page < endLimit ){
				add_to_list(ptr, page);
				page += (size/16);
			}
		}
		return pop_from_list(ptr);
	}

	else{

	}

	printf("mymalloc is not implemented\n");
	abort();
	return NULL;
}
