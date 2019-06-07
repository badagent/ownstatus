#define _POSIX_C_SOURCE 199309L
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

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
