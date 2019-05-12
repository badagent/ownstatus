#define _POSIX_C_SOURCE 199309L
#define alloca(x) __builtin_alloca(x)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include <X11/Xlib.h>

#define INTERVAL_SEC 1
#define INTERVAL_NSEC 0

//TODO Timed updates
//TODO XCloseDisplay(dpy)
//TODO Error Handling


const char* batfile = "/sys/class/power_supply/BAT0/capacity";
const char* statusfile = "/sys/class/power_supply/AC/online";

void print_timespec(struct timespec *t) {
	printf("%ld %ld\n",t->tv_sec,t->tv_nsec);
}

void diff_timespec(struct timespec *diff, struct timespec *start, struct timespec *end) {
	if((end->tv_nsec - start->tv_nsec) < 0) {
		diff->tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
		diff->tv_sec = end->tv_sec-start->tv_sec-1;
	} else {
		diff->tv_nsec = end->tv_nsec - start->tv_nsec;
		diff->tv_sec = end->tv_sec - start->tv_sec;
	}
}


int get_volume(char buffer[], int index) {
		/* ALSA */
		const char *card = "default";
		snd_mixer_t *handle;
		snd_mixer_elem_t *elem;
		snd_mixer_selem_id_t *sid;
		long min,max;
		long volume;
		long percentage;
		int switch_value;

		snd_mixer_open(&handle,0); //Open mixer handle
		snd_mixer_attach(handle,card); //Attach mixer handle to hardware(card)
		snd_mixer_selem_register(handle,NULL,NULL);
		snd_mixer_load(handle);

		snd_mixer_selem_id_alloca(&sid);
		snd_mixer_selem_id_set_index(sid,0);
		snd_mixer_selem_id_set_name(sid,"Master");

		elem = snd_mixer_find_selem(handle,sid);
		snd_mixer_selem_get_playback_volume_range(elem,&min,&max);
		snd_mixer_selem_get_playback_volume(elem,0,&volume);
		snd_mixer_selem_get_playback_switch(elem,0,&switch_value);
		percentage = (volume-min)*100/(max-min);
		snd_mixer_close(handle);	
		if(!switch_value) {
			return snprintf(buffer+index,10,"\ue202");
		}else {
			return snprintf(buffer+index,10,"\ue203%ld%%",percentage);
		}
}

int get_time(char buffer[], int index) {
	time_t current_time;
	current_time = time(NULL);	
	struct tm tm = *localtime(&current_time);
	return strftime(buffer+index,30,"%c",&tm);
}

int get_batt(char buffer[], int index) {
	FILE *fp;
	int perc;
	char buf[10];

	fp = fopen(batfile,"r");
	fgets(buf,10,fp);
	perc = atoi(buf);
	fclose(fp);

	fp = fopen(statusfile,"r");
	fgets(buf,10,fp);
	fclose(fp);
	
	if(buf[0]==0x31) {
		return snprintf(buffer+index,10,"\ue215%d%%",perc);
	} else {
		if(perc>=90) {
			return snprintf(buffer+index,10,"\ue24b%d%%",perc);
			return 0;
		}
		if(perc>=80) {
			return snprintf(buffer+index,10,"\ue24a%d%%",perc);
			return 0;
		}
		if(perc>=70) {
			return snprintf(buffer+index,10,"\ue249%d%%",perc);
			return 0;
		}
		if(perc>=60) {
			return snprintf(buffer+index,10,"\ue248%d%%",perc);
			return 0;
		}
		if(perc>=50) {
			return snprintf(buffer+index,10,"\ue247%d%%",perc);
			return 0;
		}
		if(perc>=40) {
			return snprintf(buffer+index,10,"\ue246%d%%",perc);
			return 0;
		}
		if(perc>=30) {
			return snprintf(buffer+index,10,"\ue245%d%%",perc);
			return 0;
		}
		if(perc>=20) {
			return snprintf(buffer+index,10,"\ue244%d%%",perc);
			return 0;
		}
		if(perc>=10) {
			return snprintf(buffer+index,10,"\ue243%d%%",perc);
			return 0;
		}
		return snprintf(buffer+index,10,"\ue242%d%%",perc);
	}
}

int main() {
	

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
	
	while(1) {
		clock_gettime(CLOCK_REALTIME,&start);

		int index = 0;
		strncpy(buffer+index," ",10);
		index++;
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


	XCloseDisplay(dpy);
}
