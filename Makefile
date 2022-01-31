CC=gcc -o ./bin/
CFLAGS = `pkg-config --cflags glib-2.0`
LDLIBS = `pkg-config --libs glib-2.0`

make_path:= $(abspath $(lastword $(MAKEFILE_LIST)))
path:= $(dir $(make_path))bin/

FILES:=library/main.c library/message.c library/client.c library/view.c library/table.c
SERV :=server/server.c library/message.c library/client.c library/view.c library/table.c

bin_path: 
	export PATH=$$PATH:$(path)

compile:
	$(CC)netP $(CFLAGS) $(FILES) $(LDLIBS) -lssl -lcrypto -lm 

test:
	$(CC)netS $(CFLAGS) $(SERV) $(LDLIBS) -lssl -lcrypto -lm 

clean:
	rm -rf *.out *.o

install: 
	sudo apt-get install libssl-dev openssl miredo pkg-config glib2.0 -y