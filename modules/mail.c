#include <dirent.h>
#include <stdio.h>
#include "mail.h"

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
