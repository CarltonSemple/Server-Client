#ifndef __CLIENT_H_INCLUDE__
#define __CLIENT_H_INCLUDE__

#define responseSize	// size of fifo response

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include "data_t.h"	// message queue struct

// Read the response
void *readFifo(void *fifoNameArg)
{
	int serverFifoFd, numRead;
	data_t response = {};	// fifo response
	
	serverFifoFd = open((char*)fifoNameArg, O_RDONLY);	// open the fifo
		
	for(;;)	// keep reading
	{		
		// Read, read, read...
		numRead = read(serverFifoFd, &response, sizeof(data_t));
		if(numRead != sizeof(data_t))
		{
			continue;	// continue even if there was an error or partial read
		}
		else
		{	// successful read
			//printf("%d read\n", sizeof(response));
			printf("%s\n", response.word);
		}
		
		//close(serverFifoFd);
	}
}

// Check for server quit and exit commands
// -1 = exit
// 1 = server quit
int checkInput(char *stri)
{
	char *substring = malloc(4 * sizeof(char));	// used to see if exit was entered
	
	strncpy(substring, stri, 4);	// copy the first 4 characters to check for exit
	substring[4] = '\0';					// add null character manually
	
	// exit command
	if(strcmp(substring, "exit") == 0)
		return -1;				// exit	
	
	free(substring);		// free substring to re-allocate its size
	
	// Check for server quit command
	substring = malloc(11 * sizeof(char));	// server quit is 11 characters
	strncpy(substring, stri, 11);
	substring[11] = '\0';
	
	if(strcmp(substring, "server quit") == 0)
		return 1;
	
	return 0;
}

int clientMain(char *fifoName)
{
	// prepare for response
	int fifoFd, readLength, serverQuit = 0;
	pthread_t fifeThread;
		
	mqd_t mqd;				// message queue file descriptor
	int flags = O_WRONLY;	// write flag for queue
	data_t request = {};	// request to be sent via message queue
	char queueName[13] = "/";	// prepare to build queue name from fifo name. must start with /
	
	int nbytes = 100;	// used in getline
	char *stri = NULL;
	char *substring = NULL;	// used to check for exit
		
	// Read from the server fifo on a separate process
	fifeThread = pthread_create(&fifeThread, NULL, &readFifo, 
											(void*) fifoName);	// arguments

	// MESSAGE QUEUE **************
	strcat(queueName, fifoName);		// build message queue name
	
	mqd = mq_open(queueName, flags); 	// open message queue for writing
	
	
	for(;;)		// continuously accept user input
	{
		request.pid = getpid();	// give request the process ID in case it's needed
		readLength = getline(&stri, &nbytes, stdin); 	// get the user command
		
		strcpy(request.word, stri);
		
		switch(checkInput(stri))	// check for exit or server quit
		{
		case -1:
			exit(1);
			break;
		case 1:
			serverQuit = 1;
			break;
		default:
			break;
		}
		
		if(serverQuit == 0)	// don't try to send to a terminated server
		{
			if(mq_send(mqd, (char*)&request, sizeof(request), 0) == -1)		// send the request via the message queue
			{
				printf("Error message send\n");
				exit(1);
			}
		}
	}
	

	
}

#endif
