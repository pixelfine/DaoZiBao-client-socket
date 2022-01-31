
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <glib.h>
#include <gmodule.h>

#include "view.h"
#include "message.h"
#include "table.h"




void viewInterface(GHashTable * received){
	printf("\n\n\n\n*****************INTERFACE*********************\n\n");
	printf("\33[0;%sm", BOLD_YELLOW); printf("\t\t大字报２\n"); printf("size : %d\n", tableSize(received)); printf("\033[0m");
	g_hash_table_foreach(received, printReceived, NULL);
	printf("\n\n*****************INTERFACE*********************\n\n");

	printf("\33[0;%sm", BOLD_YELLOW); 
	printf("Vous êtes dans l'interface de données reçus\n");
	printf("\033[0m\n");

	printf("\33[0;%sm", BOLD_BLUE); 
	printf("[#console] pour visionner la console\n[#receive] pour visionner l'interface des données reçus\n[#publish] pour aller au mode de publication\n[#refresh] pour rafraîchir le mode actuel\n[#exit] pour quitter le programme\n"); 
	printf("\033[0m\n");
}

void viewPublished(GHashTable * published){
	printf("\n\n\n\n*****************INTERFACE*********************\n\n");
	printf("\33[0;%sm", BOLD_YELLOW); printf("\t\t大字报２_published\n"); printf("size : %d\n", tableSize(published)); printf("\033[0m");
	g_hash_table_foreach(published, printPublished, NULL);
	printf("\n\n*****************INTERFACE*********************\n\n");

	printf("\33[0;%sm", BOLD_YELLOW); 
	printf("Vous êtes dans l'interface de données publiées\n");
	printf("\033[0m\n");

	printf("\33[0;%sm", BOLD_BLUE);
	printf("[#console] pour visionner la console\n[#receive] pour visionner l'interface des données reçus\n[#publish] pour aller au mode de publication\n[#refresh] pour rafraîchir le mode actuel\n[#exit] pour quitter le programme\n"); 
	printf("\n[#post] pour créer une nouvelle publication \n"); 
	printf("[#edit] pour éditer une publication \n");
	printf("[#del][id] pour supprimer une publication \n");
	printf("\033[0m\n");
}

void viewConsole(){
	printf("\33[0;%sm", BOLD_YELLOW); 
	printf("Vous êtes dans la console\n");
	printf("\033[0m\n");

	printf("\33[0;%sm", BOLD_BLUE); 
	printf("[#console] pour visionner la console\n[#receive] pour visionner l'interface des données reçus\n[#publish] pour aller au mode de publication\n[#refresh] pour rafraîchir le mode actuel\n[#exit] pour quitter le programme\n"); 
	printf("\033[0m\n");
}

void viewPost(){
	printf("\33[0;%sm", BOLD_YELLOW); 
	printf("\nEntrez une succession de ligne de format [nombre][data], nombre sera le timeout en seconde. \nUne fois terminé, entrez [#end] pour publier \nPar exemple :1212Hello world\n#end"); 
	printf("\033[0m\n");
}

void viewEdit(){
	printf("\33[0;%sm", BOLD_YELLOW); 
	printf("\nEntrez une ligne de format [id][données] afin de modifier la donnée associé à l'id.\nExemple :<<MyId>>Hello world"); 
	printf("\033[0m\n");
}