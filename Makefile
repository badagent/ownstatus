all: ownstatus

ownstatus: ownstatus.c
	$(CC) ownstatus.c -o ownstatus -lX11 -lasound -Wall -Wextra -pedantic -std=c99

clean: 
	rm ownstatus

install: all
	mkdir -p ~/bin/
	cp ownstatus ~/bin/
