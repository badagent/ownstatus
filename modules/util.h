#define _POSIX_C_SOURCE 199309L
#ifndef UTIL_H 
#define UTIL_H 

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define INTERVAL_SEC 1
#define INTERVAL_NSEC 0

void send_notification(char title[], char msg[], char urgency[]);
void diff_timespec(struct timespec *diff, struct timespec *start, struct timespec *end);

#endif
