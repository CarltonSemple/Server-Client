#ifndef __SERVER_H_INCLUDE__
#define __SERVER_H_INCLUDE__

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "data_t.h"	// message structure

#define BUFF 100

// Execute the shell command that was determined to have been sent in the request
void shellCommandExecute(int *fifoFd, data_t *request, data_t *response)
{
	FILE *fp;				// for use in popen()
	char readLine[BUFF];
	
	char *command = malloc(94 * sizeof(char));		// the size of request->word - 6. ("shell " = 6 char)
	
	strncpy(command, request->word + 6, 94);		// copy the request message without "shell "
													// null character will already be in the string
	
	fp = popen(command, "r");						// execute shell command
	if(fp == NULL)
		printf("shell command execution failed\n");

	while(fgets(readLine, BUFF * sizeof(char), fp))		// get results of command
	{
		//printf("%s", readLine);
		strcat(response->word, readLine);
	}
	
	if(pclose(fp) == -1)					// close file stream
		printf("Error pclose\n");	
}

// Quit command returns -1; help command returns 1
// Shell command returns 2
int processCommand(data_t *request)
{
	// commands are "quit", "help", "shell".. the last of which is followed by a shell command
	char *substring = malloc(11 * sizeof(char));	// used to see if server quit is sent
	
	strncpy(substring, request->word, 11);	// copy the first 11 characters to check for server quit
	substring[11] = '\0';					// add null character manually
	
	// quit command
	if(strcmp(substring, "server quit") == 0)
		return -1;				// quit	
		
	free(substring);
		
	substring = malloc(4 * sizeof(char));	// used to see if help is sent	
	strncpy(substring, request->word, 4);	// copy the first 4 characters to check for help
	substring[4] = '\0';					// add null character manually
		
	// help command
	if(strcmp(substring, "help") == 0)
		return 1;				// help
	
	free(substring);		// free substring to re-allocate its size
	
	// Check for shell command
	substring = malloc(5 * sizeof(char));	// shell is 5 characters
	strncpy(substring, request->word, 5);
	substring[5] = '\0';
	
	if(strcmp(substring, "shell") == 0)
		return 2;				// shell command
	
	return 0;
}

int serverMain()
{
	int sFifoD;	// server fifo descriptor
	int fifoWriter;	// server file descriptor for writing
	mqd_t mqd;	// message queue descriptor
	struct mq_attr attr = {};	// to be used in the message queue
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(data_t);
	data_t requestData = {};	// to hold data from message queue
	ssize_t numRead;	
	// end message queue info
	char fifoName[] = "sempleXXXXXX";	// template for fifo name
	char queueName[13] = "/";			// prepare to build queue name from fifo name. must start with /
	
	int quitFlag = 0, defaultFlag = 0;	// used in the loop
	
	// Response data ****************************
	data_t responseData = {};			// what's sent to the client over the fifo
	responseData.pid = getpid();
	char helpString[] = "Server FIFO: %s\nPID: %d\n";	// to be written to the fifo
	
	
	// FIFO **************************************
	mkstemp(fifoName);	// get unique fifo name
	
	// Create a fifo
	mkfifo(fifoName, S_IRUSR | S_IWUSR | S_IWGRP);	// create unique fifo
	
	printf("Server FIFO: %s\n", fifoName);		// print fifo name
	
	// ************** MESSAGE QUEUE **************
	
	strcat(queueName, fifoName);		// build message queue name
	mqd = mq_open(queueName, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, &attr);	// create message queue
	
	fifoWriter = open(fifoName, O_WRONLY);			// open fifo for writing
	
	for(;;)
	{
		// Might have to wait for message notification
		
		numRead = mq_receive(mqd, (char*)&requestData, sizeof(requestData), NULL);	// Read message queue. Read amount (message size) is consistent 
																		// between server & client due to data_t structure.
											// mq_receive() blocks (waits) until a message is available since O_NONBLOCK isn't being used.		
		
		// Process shell command from user
		switch(processCommand(&requestData))
		{
		case -1:	// quit
			quitFlag = 1;
			break;	
		case 1:		// help
			snprintf(responseData.word, 100, "Server FIFO: %s\nPID: %d\n", fifoName, getpid());	// Format the response's text
			break;
		case 2:		// shell command
			shellCommandExecute(&fifoWriter, &requestData, &responseData);	// execute request's shell command. store results in responseData	
			break;
		default:	// do nothing or print an error
			strcpy(responseData.word, "Proper commands: \nserver quit\nhelp\nshell command\n");
			break;
		}
		
		if(quitFlag == 1)
			break;
		
		if(write(fifoWriter, &responseData, sizeof(data_t)) != sizeof(data_t))	// write response data to fifo
			printf("Error writing to fifo\n");
			
		strcpy(responseData.word, "");	// reset response message
		
	}
	
	// send message to stop reading, to client
	strcpy(responseData.word, "server terminated");
	if(write(fifoWriter, &responseData, sizeof(data_t)) != sizeof(data_t))	// write response data to fifo
		printf("Error writing to fifo\n");
	
	// close the fifo
	unlink(fifoName);
	
	if(close(fifoWriter) == -1){
		printf("Error closing fifo\n");
	}
	
	if(mq_close(mqd) < 0)	// close message queue
		printf("Error closing message queue\n");
	
	return 0;
}

#endif
