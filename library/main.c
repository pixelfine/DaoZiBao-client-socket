#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <endian.h>

#include <poll.h>
#include <unistd.h>

#include "message.h"
#include "view.h"
#include "client.h"

int main(int argc, char **argv){
	printf("*****************Starting************************\n\n\n");
	srand(time(NULL));

	run();

	printf("\n\n\n*****************Closing*************************\n");
}