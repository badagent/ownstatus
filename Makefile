all: ownstatus

ownstatus: ownstatus.c modules/mail.c modules/battery.c modules/mpd.c modules/alsa.c modules/util.c modules/time.c
	$(CC) ownstatus.c modules/mpd.c modules/mail.c modules/battery.c modules/alsa.c modules/util.c modules/time.c -o ownstatus -lmpdclient -lX11 -lasound -Wall -Wextra -pedantic -std=c99

clean: 
	rm ownstatus

install: all
	mkdir -p ~/bin/
	cp ownstatus ~/bin/
