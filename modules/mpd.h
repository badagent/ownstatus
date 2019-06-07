#ifndef MPD_H
#define MPD_H

#include <mpd/client.h>
#include <stdlib.h>
#include <stdio.h>

void MPD_init(char server[], int port);
void MPD_deinit();
int MPD_get_status(char buffer[], int index);
int MPD_get_status_and_song(char buffer[], int index);

#endif
