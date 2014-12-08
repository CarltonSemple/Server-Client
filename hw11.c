#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "server.h"
#include "client.h"


int main(int argc, char **argv)
{	
	int option, c = 0, s = 0;	// options
	int i = 1;
	
	// options. -c will be client. -s will be server
	while ((option = getopt(argc, argv, "cs")) != -1)
		switch(option)
		{
		case 'c':
			c = 1;
			break;
		case 's':
			s = 1;
			break;
		default:
			printf("Something went wrong. -s for server\n-c for client\n");
			break;
		}
		
	// server mode
	if(s == 1)
		serverMain();
	else if(c == 1)	// client mode
	{
		if(argc == 3)	// require the server fifo name as an argument
			clientMain(argv[optind]);
		else
			printf("Expected: -c serverName\n");
	}
	else
		printf("-s : server . -c : client\n");

}
