#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stddef.h>

void *mymalloc(size_t Size);
void myfree(void *ptr);
long long int* free_list[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int sizeRef[] = {16,32,64,128,256,024,2048,4096};

#endif
