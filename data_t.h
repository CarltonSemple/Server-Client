#ifndef __DATA_T_H_INCLUDE__
#define __DATA_T_H_INCLUDE__

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct{
	pid_t pid;
	char word[1000];
}data_t;	// message queue data structure

#endif
