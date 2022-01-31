#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <glib.h>
#include <gmodule.h>
	
	#define RED 		 "0;31";
	#define BOLD_RED 	 "1;31"
	#define GREEN 		 "0;32"
	#define BOLD_GREEN 	 "1;32"
	#define YELLOW 		 "0;33"
	#define BOLD_YELLOW  "1;33"
	#define BLUE 		 "0;34"
	#define BOLD_BLUE 	 "1;34"
	#define MAGENTA 	 "0;35"
	#define BOLD_MAGENTA "1;35"
	#define CYAN 		 "0;36"
	#define BOLD_CYAN 	 "1;36"

	typedef struct addrinfo addrinfo;
	typedef struct sockaddr sockaddr;

	typedef struct{
		uint8_t magic;
		uint8_t version;
		uint16_t bodylength;
	}Head;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint8_t mbz[];
	}PadN;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint16_t timeout;
		char tag [4];
		char id [8];
		char data[];
	}Notify;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint8_t mbz[2];
		char tag [4];
		char id [8];
	}Notify_Ack;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint16_t timeout;
		char id [8];
		char secret [8];
		char data[];
	}Publish;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint8_t mbz[2];
		char tag[4];
	}Dump;

	typedef struct{
		uint8_t type;
		uint8_t length;
		char message[];
	}Warning;

	typedef struct{
		uint8_t type;
		uint8_t length;
		uint8_t mbz[2];
		char tag[4];
	}Dump_Ack;
	//prepare
	char * setHead(char * buf, uint16_t bodylength);
	char * setPad1(char * buf, int idx);
	char * setPadN(char * buf, int idx, uint8_t length);
	char * setNotify_Ack(char * buf, int idx, char * tag, char * id);
	char * setPublish(char * buf, int idx, uint8_t length, uint16_t timeout, char * id, char * secret, char * data);
	char * setDump(char * buf, int idx, char * tag);
	bool isHead(char * buf);
	//debugging
	uint16_t printTlv(uint16_t bodylength, char * buf);
	void printMsg(char * buf, char * color);
	//envoies
	bool sendHead(int sockfd, addrinfo * addr);
	bool sendDump(int sockfd, addrinfo * addr, char * tag);
	bool sendNotifyAck(int sockfd, addrinfo * addr, char * tag, char * id);
	bool sendPublish(int sockfd, addrinfo * addr, uint16_t timeout, char * id, char * secret, size_t length, char * data);

	void onReceive(int sockfd, addrinfo * addr, char * buf, GHashTable * published, GHashTable * received);
	size_t onReceivedTlv(uint16_t bodylength, char * buf, char * sendBuf, size_t sendLen, GHashTable * published, GHashTable * received);




#endif 


	/*
	Client				|Serveur
	-----------------------------
						|
	Demarrage____________________
	Dump				>Notify
						|
						|
	Notification_________________
						<Notify
						|
						|
						|
						|
						|
						|
						|
						|
						|
						|
						|




	*/