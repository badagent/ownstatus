#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "battery.h"

const char* batfile = "/sys/class/power_supply/BAT0/capacity";
const char* statusfile = "/sys/class/power_supply/AC/online";

int firstrun = TRUE;
int charging;
int last_battery_charge;

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
		charging = TRUE;
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
		charging = FALSE;
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
	firstrun = FALSE;
}

