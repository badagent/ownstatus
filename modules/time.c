#include <time.h>
#include <locale.h>
#include "time.h"


int get_time(char buffer[], int index) {
	time_t current_time;
	current_time = time(NULL);	
	struct tm tm = *localtime(&current_time);
	return strftime(buffer+index,40,"%a %x KW%W %H:%M:%S",&tm);
}
