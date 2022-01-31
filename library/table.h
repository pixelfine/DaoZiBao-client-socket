#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <gmodule.h>
#include <stdint.h>
#include <time.h>

	typedef struct{
		char id[8];
		uint16_t timeout;
		time_t timestamp;
		uint8_t length;
		char data[];
	}ReceivedData;

	typedef struct{
		char id[8];
		uint16_t timeout;
		uint16_t refreshDate;
		time_t timestamp;     //unsigned long
		char secret[8];
		uint8_t length;
		char data[];
	}PublishedData;

	typedef struct tm tm;


	ReceivedData *  newReceivedData(char * id, uint16_t timeOut, uint8_t length, char * data);
	PublishedData *   newPublishedData (char * id, uint16_t timeOut, char * secret, uint8_t length, char * data);

	guint 		 tableSize(GHashTable * table);
	gboolean     publish_add(GHashTable *table, char * key, PublishedData * value);
	gboolean     receive_add(GHashTable *table, char * key, ReceivedData * value);
	gboolean 	 del_item(GHashTable * table, char * key);
	gboolean 	 table_contains(GHashTable * table, char * key);
	gboolean 	 itemEqual (const void * k1, const void * k2);
	gboolean 	 isExpiredReceived(gpointer key, gpointer value, gpointer data);
	PublishedData *  published_get(GHashTable *table, char * key);
	ReceivedData *   received_get(GHashTable *table, char * key);

	bool addPublishedList(char * buf, GHashTable * table);

	void         printReceived(gpointer key, gpointer value, gpointer user_data);
	void 		 printPublished(gpointer key, gpointer value, gpointer user_data);

	time_t 		timeLeft(time_t timestamp, uint16_t timeout);
	char * fillRand(char * buf, size_t length);

	char * cpyId(char * id);


	//(TAG, ID) => identifie une notification de façon unique

#endif 