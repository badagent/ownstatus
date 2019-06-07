#define _POSIX_C_SOURCE 199309L
#define alloca(x) __builtin_alloca(x)

#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpd/client.h>

#include <time.h>
#include <locale.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include <X11/Xlib.h>

#define INTERVAL_SEC 1
#define INTERVAL_NSEC 0
#define BATTERY_LOW_THRESHOLD 16
//TODO Error Handling


const char* batfile = "/sys/class/power_supply/BAT0/capacity";
const char* statusfile = "/sys/class/power_supply/AC/online";


struct mpd_connection *conn;
Bool firstrun = True;
Bool charging;
int last_battery_charge;

void print_timespec(struct timespec *t) {
	printf("%ld %ld\n",t->tv_sec,t->tv_nsec);
}


void send_notification(char title[], char msg[], char urgency[]) {
	pid_t pid;
	switch( pid = fork()) {
		case -1:
			printf("Error while forking for send_notification()\n");
			break;
		case 0:
			execl("/usr/bin/notify-send","notify-send","-u",urgency,title,msg,0);
			exit(0);
			break;
		default:
			break;
	}
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


int get_mails_in_dir(char directory[], char buffer[], int index) {
	DIR *dp;
	struct dirent *ep;
        int count = 0;	
	dp = opendir(directory);
	if(dp != NULL) {
		while((ep = readdir(dp)))
			count++;
		closedir(dp);
		count -= 2; 
		return snprintf(buffer+index,10,"\uf0e0 %d",count);
	}
	else {
		return 0;
	}	
}

int get_mpd_status(char buffer[], int index) {

	if(conn==NULL) conn = mpd_connection_new("127.0.0.1",6600,0);
	if(conn==NULL) return 0;
	

	if(!mpd_command_list_begin(conn,true) ||
	   !mpd_send_status(conn) ||
	   !mpd_command_list_end(conn)) {
		return 0;
	};
	
	struct mpd_status *status = mpd_recv_status(conn);
	if(status==NULL)  {
		return 0;
	}

	switch(mpd_status_get_state(status)) {
		case MPD_STATE_UNKNOWN:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue099");
			break;
		case MPD_STATE_STOP:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue099");
			break;
		case MPD_STATE_PAUSE:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue09b");
			break;
		case MPD_STATE_PLAY:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue058");
			break;
	}
	return 0;
}


int get_mpd_status_and_song(char buffer[], int index) {

	if(conn==NULL) conn = mpd_connection_new("127.0.0.1",6600,0);
	if(conn==NULL) return 0;
	

	if(!mpd_command_list_begin(conn,true) ||
	   !mpd_send_status(conn) ||
	   !mpd_send_current_song(conn) ||
	   !mpd_command_list_end(conn)) {
		return 0;
	};
	
	struct mpd_status *status = mpd_recv_status(conn);
	if(status==NULL)  {
		return 0;
	}

	int written = 0;
	switch(mpd_status_get_state(status)) {
		case MPD_STATE_UNKNOWN:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue099");
			break;
		case MPD_STATE_STOP:
			mpd_status_free(status);
			return snprintf(buffer+index,10,"\ue099");
			break;
		case MPD_STATE_PAUSE:
			mpd_response_next(conn);
			written += snprintf(buffer+index,10,"\ue09b");
			break;
		case MPD_STATE_PLAY:
			mpd_response_next(conn);
			written += snprintf(buffer+index,10,"\ue058");
			break;
	}

	struct mpd_song *song = mpd_recv_song(conn);
	if(song != NULL) {
		written = snprintf(buffer+index+written,100,"%s - %s",mpd_song_get_tag(song,MPD_TAG_ARTIST,0),mpd_song_get_tag(song,MPD_TAG_TITLE,0));
		mpd_song_free(song);
	}

	mpd_status_free(status);
	return written;
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
			return snprintf(buffer+index,10,"\uf6a9");
		}else {
			return snprintf(buffer+index,10,"\uf028%ld%%",percentage);
		}
}

int get_time(char buffer[], int index) {
	time_t current_time;
	current_time = time(NULL);	
	struct tm tm = *localtime(&current_time);
	return strftime(buffer+index,40,"%a %x KW%W %H:%M:%S",&tm);
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
		if(!firstrun && !charging) {
			send_notification("Status","Plugged in","low");
		}
		charging = True;
		last_battery_charge = perc;
		return snprintf(buffer+index,10,"\uf1e6%d%%",perc);
		} else {

		if(!firstrun && perc < BATTERY_LOW_THRESHOLD && (last_battery_charge == BATTERY_LOW_THRESHOLD || charging)) {
			send_notification("Status","Batter low.","Critical");
		}
		last_battery_charge = perc;
		if(!firstrun && charging) {
			send_notification("Status","Not plugged in","low");
		}
		charging = False;
		if(perc>=90) {
			return snprintf(buffer+index,10,"\uf240%d%%",perc);
			return 0;
		}
		if(perc>=70) {
			return snprintf(buffer+index,10,"\uf241%d%%",perc);
			return 0;
		}
		if(perc>=40) {
			return snprintf(buffer+index,10,"\uf242%d%%",perc);
			return 0;
		}
		if(perc>=10) {
			return snprintf(buffer+index,10,"\uf243%d%%",perc);
			return 0;
		}
		return snprintf(buffer+index,10,"\uf244%d%%",perc);
	}
}

int main() {



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
		index += get_volume(buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_batt(buffer,index);
		strncpy(buffer+index," | ",10);
		index+=3;
		index += get_time(buffer,index);
		XStoreName(dpy,DefaultRootWindow(dpy),buffer);
		XFlush(dpy);

		firstrun = False;

		clock_gettime(CLOCK_REALTIME,&end);

		diff_timespec(&diff,&start,&end);

		diff_timespec(&wait,&diff,&interval);
		if(wait.tv_sec>=0) {
			nanosleep(&wait,NULL);
		}
	}


	if(conn!=NULL) mpd_connection_free(conn);
	XCloseDisplay(dpy);
}
