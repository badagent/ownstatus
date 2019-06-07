#ifndef BATTERY_H
#define BATTERY_H

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define BATTERY_LOW_THRESHOLD 16

#define TRUE 1
#define FALSE 0

int get_batt(char buffer[], int index);

#endif
