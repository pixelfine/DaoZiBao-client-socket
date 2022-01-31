#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

	#define HOST "vps-63c87489.vps.ovh.net"
	#define PORT "1212"

	#define CONSOLE 0
	#define PUBLISH 1
	#define INTERFACE 2

	typedef struct addrinfo addrinfo;
	typedef struct sockaddr sockaddr;
	typedef struct{
		int socket;
		addrinfo * addr;
	}sock_addr;
	
	int cmd;

	void run();
	addrinfo * setAddrInfo(int ai_flags, int ai_family, int ai_socktype, int ai_protocol);
	void onLineInserted(int sockfd, addrinfo * addr, char * cmdLine, char * publishBuf, size_t * padding, GHashTable * published, GHashTable * received);
	int indexOfNonDigit(char * buf);
	bool update_tables(int sockfd, addrinfo * addr, GHashTable * received, GHashTable * published);


#endif 