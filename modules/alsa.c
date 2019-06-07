#define _POSIX_C_SOURCE 199309L
#define alloca(x) __builtin_alloca(x)

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include "alsa.h"

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
