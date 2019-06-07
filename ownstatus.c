#define _POSIX_C_SOURCE 199309L
#define alloca(x) __builtin_alloca(x)

#include "modules/util.h"

#include "modules/mail.h"
#include "modules/battery.h"
#include "modules/alsa.h"
#include "modules/mpd.h"
#include "modules/time.h"

#include <time.h>
#include <locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>


int marquee_index = 0;

int roll_text(char text[], int begin_at, int chars_to_write, char buffer[], int index) {
	int textlen = strlen(text);	
	int cur_char;
	for(int i=0;i<chars_to_write;i++) {
		cur_char = (begin_at + i)%textlen;
		buffer[index+i] = text[cur_char];
	}
	return chars_to_write;
}

int main() {
	

	MPD_init("localhost",6600);

	// Set Locale	
	const char* LANG = getenv("LANG");
	if(LANG) {
		setlocale(LC_TIME,LANG);	
	}

	/** X11 **/
	Display *dpy;

	/** Status Bar **/
	struct timespec start,end,interval,wait,diff;
	char buffer[255];

	//Init X
	dpy = XOpenDisplay(NULL);

	//Update interval
	interval.tv_sec =INTERVAL_SEC;
	interval.tv_nsec = INTERVAL_NSEC;

	send_notification("Hi","Welcome back!","low");	
	while(1) {
		clock_gettime(CLOCK_REALTIME,&start);

		int index = 0;
		strncpy(buffer+index," ",10);
		index++;
		index += get_mails_in_dir("/home/badagent/Mail/gmail/INBOX/new",buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_mails_in_dir("/home/badagent/Mail/acht-werk/INBOX/new",buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += MPD_get_status(buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_volume(buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_batt(buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_time(buffer,index);
		XStoreName(dpy,DefaultRootWindow(dpy),buffer);
		XFlush(dpy);

		clock_gettime(CLOCK_REALTIME,&end);

		diff_timespec(&diff,&start,&end);

		diff_timespec(&wait,&diff,&interval);
		if(wait.tv_sec>=0) {
			nanosleep(&wait,NULL);
		}
	}

	MPD_deinit();
	XCloseDisplay(dpy);
}
