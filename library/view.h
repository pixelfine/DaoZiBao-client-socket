#ifndef VIEW_H_
#define VIEW_H_

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gmodule.h>

void viewInterface(GHashTable * received);
void viewPublished(GHashTable * published);
void viewConsole();
void viewPost();
void viewEdit();

#endif 
