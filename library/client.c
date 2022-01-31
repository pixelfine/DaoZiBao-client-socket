#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <glib.h>
#include <gmodule.h>
#include <poll.h>
#include <unistd.h>
#include <ctype.h>

#include "client.h"
#include "message.h"
#include "table.h"
#include "view.h"

int cmd = 0;
bool quit=false;

void run(){
	struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
	char * cmdLine = malloc(1024);
	time_t cycle = time(NULL);
	time_t assocTime = time(NULL);
	char * publishBuf = calloc(1024, sizeof(char));
	size_t * padding = malloc(sizeof(size_t));
	padding[0]=sizeof(Head);


	char * buf = calloc(1024, sizeof(char));
	GHashTable * published = g_hash_table_new_full(g_int_hash, itemEqual, g_free, g_free);
	GHashTable * received  = g_hash_table_new_full(g_int_hash, itemEqual, g_free, g_free);

	int sockfd;
	int rc;
	int res;
	addrinfo * hints = setAddrInfo(AI_PASSIVE, AF_UNSPEC, SOCK_DGRAM, 0); 

	addrinfo * result, * rp;
	
	//if(sockfd<0) {perror("socket error");  exit(1);}

	if(rc=getaddrinfo(HOST, PORT, hints, &result) != 0) {perror("invalid_address"); exit(1);}
	for(rp = result; rp!=NULL; rp=rp->ai_next){
		if(rp->ai_family == AF_INET){
			printf("IPV4\n");
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			break;
		}else if(rp->ai_family == AF_INET6){
			printf("IPV6\n");
			sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
			break;
		}else{
			printf("UNKNOWN, continue la recherche\n");
		}
	}

	if(rp!=NULL){
		if(sendDump(sockfd, rp, "heya")){
			printf("success\n");
		}

		while(!quit){
			if(res = recvfrom(sockfd, buf, 1024, MSG_DONTWAIT, rp->ai_addr, &(rp->ai_addrlen))>0){
				printMsg(buf, GREEN);
				onReceive(sockfd, rp, buf, published, received);
				//break; 
			}
			if( poll(&mypoll, 1, 20) ){
		    	fgets(cmdLine, 1024, stdin);
		        //printf("Inserted - %s\n", cmdLine);
		        onLineInserted(sockfd, rp, cmdLine, publishBuf, padding, published, received);
		    }
		    if(difftime(time(NULL), cycle)>8){
		    	cycle=time(NULL);
		    	if(tableSize(published)>0){
		    		//printf("checking publish\n");
		    		if(update_tables(sockfd, rp, received, published)){
		    			assocTime = time(NULL);
		    		}
		    	}
		    	g_hash_table_foreach_remove(received, isExpiredReceived, NULL);
		    }
		    if(difftime(time(NULL), assocTime)>50){
		    	if(sendHead(sockfd, rp)){
		    		assocTime=time(NULL);
		    	}
		    }
		}
	}else{
		printf("Mauvais port ou adresse\n");
	}
	freeaddrinfo(hints);
	freeaddrinfo(result);
	free(cmdLine);
	free(publishBuf);
	free(padding);
	free(buf);
	g_hash_table_destroy(published);
	g_hash_table_destroy(received);
}

bool update_tables(int sockfd, addrinfo * addr, GHashTable * received, GHashTable * published){
	GList * keys = g_hash_table_get_keys(published);
	GList * curr = keys;
	char * k;
	PublishedData * pub;
	ReceivedData * rec;
	time_t diffT=0;
	while(curr!=NULL){
		k = (char *)curr->data;
		pub = published_get(published, k);
		if(timeLeft(pub->timestamp, pub->refreshDate) <=0){
			printf("Effacement dans %ld\n", timeLeft(pub->timestamp, pub->timeout));
			if(timeLeft(pub->timestamp, pub->timeout) <=0){
				del_item(published, k);
				del_item(received, k);
			}else{
				if(sendPublish(sockfd, addr, pub->timeout, pub->id, pub->secret, pub->length, pub->data)){
					pub->timestamp = time(NULL);
					g_list_free(keys);
					return true; //car 1 publish par cycle de 8 sec
				}
			}
		}
		if(g_hash_table_contains(received, k)){
			rec = received_get(received, k);
			diffT = timeLeft(pub->timestamp, pub->timeout) - timeLeft(rec->timestamp, rec->timeout);
			if(diffT>2 || diffT<-2 ){
				if(sendPublish(sockfd, addr, timeLeft(pub->timestamp, pub->timeout), pub->id, pub->secret, pub->length, pub->data)){
					g_list_free(keys);
					return true; //car 1 publish par cycle de 8 sec
				}
			}
		}else{
			if(pub!=NULL){
				if(sendPublish(sockfd, addr, timeLeft(pub->timestamp, pub->timeout), pub->id, pub->secret, pub->length, pub->data)){
					g_list_free(keys);
					return true; //car 1 publish par cycle de 8 sec
				}
			}
		}

		curr=curr->next;
	}
	g_list_free(keys);
	return false;
}




addrinfo * setAddrInfo(int ai_flags, int ai_family, int ai_socktype, int ai_protocol){
	addrinfo * this = malloc(sizeof(addrinfo));
	this->ai_flags     = ai_flags;
	this->ai_family    = ai_family;
	this->ai_socktype  = ai_socktype;
	this->ai_protocol  = ai_protocol;
	this->ai_addrlen   = 0;
	this->ai_addr      = NULL;
	this->ai_canonname = NULL;
	this->ai_next      = NULL;
	return this;
}

void onLineInserted(int sockfd, addrinfo * addr, char * cmdLine, char * publishBuf, size_t * padding, GHashTable * published, GHashTable * received){
	Publish * publish = (Publish *)publishBuf+padding[0];
	PublishedData * edit=NULL;
	PublishedData * edited = NULL;
	char * id;
	int idx =0;
	if      (strncmp(cmdLine, "#console", 8)==0){
		viewConsole();
		cmd = 0;
	}else if(strncmp(cmdLine, "#receive", 8)==0){
		cmd = 1;
		viewInterface(received);
	}else if(strncmp(cmdLine, "#publish", 8)==0){
		viewPublished(published);
		cmd = 2;

	}else if(strncmp(cmdLine, "#refresh", 8)==0){
		printf("refreshing\n");
		switch(cmd){
			case 0 : viewConsole(); break;
			case 1 : viewInterface(received); break;
			case 2 : viewPublished(published); break;
			default : break;
		}
	}else if(strncmp(cmdLine, "#exit", 5)==0){
		quit=true;
	}else if(cmd == 2){
		if(strncmp(cmdLine, "#post", 5)==0){
			viewPost();
			cmd=3;
		}else if(strncmp(cmdLine, "#edit", 5)==0){
			viewEdit();
			cmd=4;
		}else if(strncmp(cmdLine, "#del", 4)==0){
			edit = published_get(published, cmdLine+4);
			if(edit!=NULL){
				if(sendPublish(sockfd, addr, 0, edit->id, edit->secret, strlen("deleted"), "deleted")){
					del_item(received, edit->id);
					del_item(published, edit->id);
					cmd=0;
					viewConsole();
				}
			}
		}
	}else if(cmd==3){
		if(strncmp(cmdLine, "#end", 4)==0){
			setHead(publishBuf, padding[0]-sizeof(Head));
			cmd=0;
			if(padding[0]>sizeof(Head)){
				if(sendto(sockfd, publishBuf, padding[0], 0, addr->ai_addr, addr->ai_addrlen) > 0 ){
					addPublishedList(publishBuf, published);
					printf("tableSize : %d\n", tableSize(published));
					printMsg(publishBuf, YELLOW);
				}else{
					printf("Echec de l'envoie\n");
				}
			}else{
				printf("Aucune donnée entrée\n");
			}
			bzero(publishBuf, padding[0]);
			padding[0]=sizeof(Head);
			viewConsole();
		}else{
			idx = indexOfNonDigit(cmdLine);
			fillRand(publish->id, 8);
			fillRand(publish->secret, 8);
			printf("data : %s\n", cmdLine+idx);
			publish->timeout = atoi(cmdLine);
			setPublish(publishBuf, padding[0], sizeof(Publish)+(strlen(cmdLine+idx))-2, publish->timeout, publish->id, publish->secret, cmdLine+idx);
			padding[0]+=sizeof(Publish)+(strlen(cmdLine)-idx);
		}
	}else if(cmd==0){
		viewConsole();
	}else if(cmd==4){
		edit = published_get(published, cmdLine);
		if(edit!=NULL){
			id = cpyId(edit->id);
			edited = newPublishedData (edit->id, htons(timeLeft(edit->timestamp, edit->timeout)), edit->secret, strlen((cmdLine+8)), cmdLine+8);
			if(sendPublish(sockfd, addr, timeLeft(edit->timestamp, edit->timeout), edit->id, edit->secret, strlen((cmdLine+8)), cmdLine+8)){
				publish_add(published, id, edited);
				cmd=0;
				viewConsole();
			}
		}
	}
}

int indexOfNonDigit(char * buf){
	size_t len = strlen(buf);
	for(int i=0; i<len; i++){
		if(!isdigit(buf[i])){
			return i;
		}
	}
	return len;
}