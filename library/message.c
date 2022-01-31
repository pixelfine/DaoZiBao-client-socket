#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <endian.h>
#include <glib.h>
#include <gmodule.h>

#include "message.h"
#include "client.h"
#include "table.h"
#include "view.h"


char * setHead(char * buf, uint16_t bodylength){
	Head * hd = (Head *)buf;
	hd->magic=95;
	hd->version = 2;
	hd->bodylength = htons(bodylength);
	return buf;
}
char * setPad1(char * buf, int idx){
	buf[idx] = (uint8_t)0;
	return buf;
}
char * setPadN(char * buf, int idx, uint8_t length){
	PadN * pad = (PadN *)(buf+idx);
	pad->type = 1;
	pad->length = length;
	for(int i=0; i<length; i++){
		pad->mbz[i]=0;
	}
	return buf;
}

char * setNotify_Ack(char * buf, int idx, char * tag, char * id){
	Notify_Ack * notify = (Notify_Ack *)(buf+idx);
	notify->type = 3;
	notify->length = 14;
	for(int i=0; i<2; i++) notify->mbz[i]=0;
	for(int i=0; i<4; i++) notify->tag[i]=tag[i];
	for(int i=0; i<8; i++) notify->id[i] =id[i];
	return buf;
}

char * setPublish(char * buf, int idx, uint8_t length, uint16_t timeout, char * id, char * secret, char * data){
	Publish * publish = (Publish *)(buf+idx);
	publish->type = 4;
	publish->length = length;
	publish->timeout= htons(timeout);
	for(int i=0; i<8; i++) publish->id[i]=id[i];
	for(int i=0; i<8; i++) publish->secret[i]=secret[i];
	int datalen = length-18;
	if(datalen>0){
		for(int i=0; i<datalen; i++) publish->data[i]=data[i];
	}
	return buf;
}

char * setDump(char * buf, int idx, char * tag){
	Dump * dump = (Dump *)(buf+idx);
	dump->type = 5;
	if(tag== NULL){
		dump->length = 0;
	}else{
		dump->length = 6;
		for(int i=0; i<2; i++) dump->mbz[i]=0;
		for(int i=0; i<4; i++) dump->tag[i]=tag[i];
	}
	return buf;
}

bool isHead(char * buf){
	Head * hd = (Head *) buf;
	if(hd->magic == 95 &&
	   hd->version == 2) 
		return true;
	return false;
}

uint16_t printTlv(uint16_t bodylength, char * buf){
	if(bodylength<=0) return 0;
	printf("__________________\n");
	uint8_t type = buf[0];
	int padding = 0;

	PadN * padn;
	Notify * notify;
	Notify_Ack * notify_ack;
	Publish * publish;
	Dump * dump;
	Warning * warning;
	Dump_Ack * dump_ack;

	int leftlen=0;

	switch(type){
		case 0 : printf("Pad1 : type :%d\n", buf[0]);
				 return printTlv(bodylength-1, buf+1);
		case 1 : padn = (PadN *)buf;
				 printf("PadN : type :%d, length :%d, mbz :", padn->type, padn->length);
				 for(int i=0; i<padn->length; i++) printf("%d", padn->mbz[i]); printf("\n");
				 padding = padn->length+2;
				 return printTlv(bodylength-padding, buf+padding);
		case 2 : notify = (Notify *)buf;
				 printf("Notify : type :%d, length :%d, timeout :%d\n\ttag :", notify->type, notify->length, htons(notify->timeout));
				 for(int i=0; i<4;i++) printf("%c", notify->tag[i]); printf("\n\tid :");
				 for(int i=0; i<8; i++) printf("%c", notify->id[i]); printf("\n\tdata :");
				 leftlen = notify->length-14;
				 for(int i=0; i<leftlen; i++) printf("%c", notify->data[i]); printf("\n");
				 padding = sizeof(Notify)+leftlen;
				 return printTlv(bodylength-padding, buf+padding);
		case 3 : notify_ack = (Notify_Ack *)buf;
				 printf("Notify_Ack : type :%d, length :%d, mbz :", notify_ack->type, notify_ack->length);
				 for(int i=0; i<2; i++) printf("%d", notify_ack->mbz[i]); printf("\n\ttag :");
				 for(int i=0; i<4; i++) printf("%c", notify_ack->tag[i]); printf("\n\tid :");
				 for(int i=0; i<8; i++) printf("%c", notify_ack->id[i]);  printf("\n");
				 padding = sizeof(Notify_Ack);
				 return printTlv(bodylength-padding, buf+padding);
		case 4 : publish = (Publish *)buf;
				 printf("Publish : type :%d, length :%d, timeout :%d\n\tid :", publish->type, publish->length, htons(publish->timeout));
				 for(int i=0; i<8; i++) printf("%c", publish->id[i]); printf("\n\tsecret :");
				 for(int i=0; i<8; i++) printf("%c", publish->secret[i]); printf("\n\tdata :");
				 leftlen = publish->length - 18;
				 for(int i=0; i<leftlen; i++) printf("%c", publish->data[i]); printf("\n");
				 padding = sizeof(Publish)+leftlen;
				 return printTlv(bodylength-padding, buf+padding);  
		case 5 : dump = (Dump *)buf;
				 printf("Dump : type :%d, length :%d", dump->type, dump->length);
				 if(dump->length==6){
				 	printf(", mbz :");
					 for(int i=0; i<2; i++) printf("%d", dump->mbz[i]); printf("\n\ttag :");
					 for(int i=0; i<4; i++) printf("%c", dump->tag[i]); 
					 padding = sizeof(Dump);
				 }else{
				 	padding = sizeof(dump->type)+sizeof(dump->length);
				 }
				 printf("\n");
				 return printTlv(bodylength-padding, buf+padding);
		case 6 : warning = (Warning *)buf;
				 printf("Warning : type :%d, length :%d\n\tmessage :", warning->type, warning->length);
				 for(int i=0; i<warning->length; i++) printf("%c", warning->message[i]); printf("\n");
				 padding = sizeof(Warning)+warning->length;
				 return printTlv(bodylength-padding, buf+padding);
		case 7 : dump_ack = (Dump_Ack *)buf;
				 printf("Dump_Ack : type :%d, length :%d, mbz :", dump_ack->type, dump_ack->length);
				 for(int i=0; i<2; i++) printf("%d", dump_ack->mbz[i]); printf("\n\ttag :");
				 for(int i=0; i<4; i++) printf("%c", dump_ack->tag[i]); printf("\n");
				 padding = sizeof(Dump_Ack);
				 return printTlv(bodylength-padding, buf+padding);
		default : printf("Invalid tlv type\n"); return 0;
	}
}

void printMsg(char * buf, char * color){
	if(cmd==CONSOLE){
		printf("\33[0;%sm", color);
		printf("\n******************Message**********************\n");
		Head * hd = (Head *) buf;
		uint16_t bodylength =0;
		if(isHead(buf)){
			bodylength = htons(hd->bodylength);
			printf("magic :%d \t version : %d \t bodylength : %d\n", hd->magic, hd->version, bodylength);
			while(bodylength>0){
				bodylength = printTlv(bodylength, buf+sizeof(Head));
			}
		}else{
			printf("Head of buf is wrong\n");
		}
		printf("\n*************Fin du Message********************\n");
		printf("\033[0m");
	}
}

bool sendHead(int sockfd, addrinfo * addr){
	size_t buflen = sizeof(Head);
	char * buf = malloc(buflen);
	setHead(buf, 0);
	if(sendto(sockfd, buf, buflen, 0, addr->ai_addr, addr->ai_addrlen) > 0){
		printMsg(buf, YELLOW);
		free(buf);
		return true;
	}free(buf);
	return false;
}

bool sendDump(int sockfd, addrinfo * addr, char * tag){
	int bodylength=sizeof(Dump);
	if(tag == NULL){
		bodylength = 2;
	}
	size_t buflen = sizeof(Head)+bodylength;
	char * buf = malloc(buflen);
	setHead(buf, bodylength);
	setDump(buf, sizeof(Head), tag);
	if(sendto(sockfd, buf, buflen, 0, addr->ai_addr, addr->ai_addrlen) > 0){
		printMsg(buf, YELLOW);
		free(buf);
		return true;
	}
	free(buf);
	return false;
}

bool sendNotifyAck(int sockfd, addrinfo * addr, char * tag, char * id){
	size_t buflen = sizeof(Head)+sizeof(Notify_Ack);
	char * buf = malloc(buflen);
	setHead(buf, sizeof(Notify_Ack));
	setNotify_Ack(buf, sizeof(Head), tag, id);

	if(sendto(sockfd, buf, buflen, 0, addr->ai_addr, addr->ai_addrlen) > 0 ){
		printMsg(buf, YELLOW);
		free(buf);
		return true;
	}
	free(buf);
	return false;
}

bool sendPublish(int sockfd, addrinfo * addr, uint16_t timeout, char * id, char * secret, size_t length, char * data){

	size_t buflen = sizeof(Head)+sizeof(Publish)+length;
	char * buf = malloc(buflen);
	setHead(buf, sizeof(Publish)+length);
	setPublish(buf, sizeof(Head), sizeof(Publish)+length-2, timeout, id, secret, data);

	if(sendto(sockfd, buf, buflen, 0, addr->ai_addr, addr->ai_addrlen) > 0 ){
		printMsg(buf, YELLOW);
		free(buf);
		return true;
	}
	free(buf);
	return false;
}

void onReceive(int sockfd, addrinfo * addr, char * buf, GHashTable * published, GHashTable * received){
	char * sendBuf = calloc(1024, sizeof(char));
	size_t sendlen = 0;

	Head * hd = (Head *) buf;
	uint16_t bodylength = 0;
	if(isHead(buf)){
		bodylength = htons(hd->bodylength);
		sendlen = onReceivedTlv(bodylength, buf+sizeof(Head), sendBuf+sizeof(Head), sizeof(Head), published, received);
		if(sendlen > sizeof(Head)){
			setHead(sendBuf, sendlen-sizeof(Head));
			if(sendto(sockfd, sendBuf, sendlen, 0, addr->ai_addr, addr->ai_addrlen) > 0){
				printMsg(sendBuf, YELLOW);
			}
		}
		
	}else{
		printf("received a wrong Head\n");
	}
	free(sendBuf);
}

size_t onReceivedTlv(uint16_t bodylength, char * buf, char * sendBuf, size_t sendLen, GHashTable * published, GHashTable * received){
	if(bodylength<=0){
		return sendLen;
	}
	uint8_t type = buf[0];
	int padding = 0;
	size_t leftlen = 0;

	Notify * notify;
	Warning * warning;
	Dump_Ack * dump_ack;
	switch(type){
		case 2 : notify = (Notify * )buf;
				 leftlen = notify->length -14; 
				 padding = sizeof(Notify) + leftlen;
				 if(receive_add(received, cpyId(notify->id), newReceivedData(notify->id, notify->timeout, leftlen, notify->data))){
				 		//printf("added ! \n");
				 		//printf("size : %d\n", tableSize(received));
				 }
				 /*if(table_contains(published, notify->id)){
				 	printf("contains\n");
				 	if(publish_add(published, cpyId(notify->id), newPublishedData(notify->id, notify->timeout, leftlen, notify->data))){
				 		printf("added ! \n");
				 	}
				 }else{
				 	printf("doesn't contains\n");
				 	if(publish_add(published, cpyId(notify->id), newPublishedData(notify->id, notify->timeout, leftlen, notify->data))){
				 		printf("added ! \n");
				 	}
				 }*/
				 setNotify_Ack(sendBuf, 0, notify->tag, notify->id);
				 return onReceivedTlv(bodylength-padding, buf+padding, sendBuf+sizeof(Notify_Ack), sendLen+sizeof(Notify_Ack), published, received);
		case 6 : warning = (Warning *)buf;
				 leftlen = warning->length;
				 padding = sizeof(Warning)+leftlen;
				 return onReceivedTlv(bodylength-padding, buf+padding, sendBuf, sendLen, published, received);
		case 7 : padding = sizeof(Dump_Ack);
				 return onReceivedTlv(bodylength-padding, buf+padding, sendBuf, sendLen, published, received);
		default : printf("Received an uncognized tlv : %d\n", type); return 0;
	}
}

