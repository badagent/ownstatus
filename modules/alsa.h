#ifndef ALSA_H
#define ALSA_H
#define _POSIX_C_SOURCE 199309L
#define alloca(x) __builtin_alloca(x)

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

int get_volume(char buffer[], int index);

#endif
