#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gmodule.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <endian.h>
#include <time.h>

#include "table.h"
#include "message.h"


ReceivedData *  newReceivedData(char * id, uint16_t timeout, uint8_t length, char * data){
	ReceivedData * this = malloc(sizeof(ReceivedData)+length);
	for(int i=0; i<8; i++) this->id[i] = id[i];
	this->timeout = htons(timeout);
	this->timestamp = time(NULL);
	this->length = length;
	for(int i=0; i<length; i++) this->data[i] = data[i];
	return this;
}

PublishedData *   newPublishedData (char * id, uint16_t timeout, char * secret, uint8_t length, char * data){
	PublishedData * this = malloc(sizeof(PublishedData) + length);
	for(int i=0; i<8; i++) this->id[i] = id[i];
	this->timeout = htons(timeout);
	this->refreshDate = (3*(this->timeout))/4;
	this->timestamp = time(NULL);
	for(int i=0; i<8; i++) this->secret[i]=secret[i];
	this->length = length;
	for(int i=0; i<length; i++) this->data[i]=data[i];
	return this;
}



guint 		 tableSize(GHashTable * table){
	return g_hash_table_size(table);
}

gboolean     publish_add(GHashTable *table, char * key, PublishedData * value){
	return g_hash_table_insert(table, key, value);
}

gboolean     receive_add(GHashTable *table, char * key, ReceivedData * value){
	return g_hash_table_insert(table, key, value);
}

gboolean 	 del_item(GHashTable * table, char * key){
	return g_hash_table_remove(table, key);
}

gboolean 	 table_contains(GHashTable * table, char * key){
	return g_hash_table_contains(table, key);
}

gboolean 	 itemEqual (const void * k1, const void * k2){
	const char * key1 = k1;
	const char * key2 = k2;
	for(int i=0; i<8; i++){
		if(key1[i]!=key2[i]){
			return false;
		}
	}
	return true;
}

gboolean 	 isExpiredReceived(gpointer key, gpointer value, gpointer data){
	char * id = key;
	ReceivedData * rec = (ReceivedData *)value;	
	if(timeLeft(rec->timestamp, rec->timeout)<0){
		return true;
	}return false;
}

PublishedData *  published_get(GHashTable *table, char * key){
	if(g_hash_table_contains(table, key)){
		return (PublishedData *)g_hash_table_lookup(table, key);
	}else{
		printf("This memory doesn't contains the key\n");
	}return NULL;
}

ReceivedData *  received_get(GHashTable *table, char * key){
	if(g_hash_table_contains(table, key)){
		return (ReceivedData *)g_hash_table_lookup(table, key);
	}else{
		printf("This memory doesn't contains the key\n");
	}return NULL;
}

/*bool replacePublished(GHashTable * table, char * key, uint8_t length, char * data){

}*/


void         printReceived(gpointer key, gpointer value, gpointer user_data){
	char * id = (char *)key;
	ReceivedData * rec = (ReceivedData *)value;
	time_t timeL = timeLeft(rec->timestamp, rec->timeout);
	tm * tm = localtime(&timeL);
	printf("\33[0;%sm", BOLD_BLUE);   
	printf("%02dh%02dm%02ds\t", tm->tm_hour-1, tm->tm_min, tm->tm_sec);
	printf("\033[0m");
	for(int i=0; i<rec->length; i++) printf("%c", rec->data[i]);
	printf("\n");
}

void 		 printPublished(gpointer key, gpointer value, gpointer user_data){
	char * id = (char *)key;
	PublishedData * pub = (PublishedData *)value;
	time_t timeL = timeLeft(pub->timestamp, pub->timeout);
	tm * tm = localtime(&timeL);
	printf("\33[0;%sm", BOLD_BLUE);   
	printf("%02dh%02dm%02ds(%ld)\t", tm->tm_hour-1, tm->tm_min, tm->tm_sec, timeL);
	printf("\033[0m");

	printf("\33[0;%sm", BOLD_CYAN);   
	for(int i=0; i<8; i++) printf("%c", pub->id[i]);
	printf("\t");
	/*for(int i=0; i<8; i++) printf("%c", pub->secret[i]);
	printf("\033[0m\t");*/

	for(int i=0; i<pub->length; i++) printf("%c", pub->data[i]);
	printf("\n");
}

time_t timeLeft(time_t timestamp, uint16_t timeout){
	time_t now = time(NULL);
	return timeout-(now-timestamp);
}


char * cpyId(char * id){
	char * k = malloc(8);
	for(int i=0; i<8; i++) k[i]=id[i];
	return k;
}

char * fillRand(char * buf, size_t length){
	for(int i=0; i<length; i++){
		buf[i]=(char) ('0'+(rand()%43));
	}
	return buf;
}

bool addPublishedList(char * buf, GHashTable * table){
	Head * head = (Head *)buf;
	char * tmpbuf = buf+sizeof(Head);
	Publish * publish = (Publish *)(tmpbuf);
	uint16_t len = htons(head->bodylength);
	uint8_t leftlen = 0;
	size_t padding = 0;
	char * id = NULL;
	PublishedData * data = NULL;
	while(len>0){
		publish = (Publish *)tmpbuf;
		leftlen = publish->length - 18;
		padding = sizeof(Publish) + leftlen;
		id = cpyId(publish->id);
		data = newPublishedData (publish->id, publish->timeout, publish->secret, leftlen, publish->data);
		if(!publish_add(table, id, data)) return false;
		len-=padding;
		tmpbuf += padding;
	}return true;
}


/*
void list_add(GPtrArray * arr, void * this){
	return g_ptr_array_add(arr, this);
}

void * list_get(GPtrArray * arr, size_t i){
	return g_ptr_array_index(arr, i);
}

void list_remove(GPtrArray * arr, size_t i){
	g_ptr_array_remove_index(arr, i);
}
*/