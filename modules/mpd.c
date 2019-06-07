#include <mpd/client.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *m_server;
int m_port;
void MPD_init(char server[], int port) {
	m_server = malloc(strlen(server)+1);
	snprintf(m_server,strlen(server)+1,server);
	m_port = port;
}

void MPD_deinit() {
	free(m_server);
}

int MPD_get_status(char buffer[], int index) {
	struct mpd_connection *conn = mpd_connection_new(m_server,m_port,0);

	if(!mpd_command_list_begin(conn,true) ||
	   !mpd_send_status(conn) ||
	   !mpd_command_list_end(conn)) {
		return 0;
	};
	
	struct mpd_status *status = mpd_recv_status(conn);
	if(status==NULL)  {
		mpd_connection_free(conn);
		return 0;
	}

	switch(mpd_status_get_state(status)) {
		case MPD_STATE_UNKNOWN:
			mpd_status_free(status);
			mpd_connection_free(conn);
			return snprintf(buffer+index,10,"\uf04d");
			break;
		case MPD_STATE_STOP:
			mpd_status_free(status);
			mpd_connection_free(conn);
			return snprintf(buffer+index,10,"\uf04d");
			break;
		case MPD_STATE_PAUSE:
			mpd_status_free(status);
			mpd_connection_free(conn);
			return snprintf(buffer+index,10,"\uf04c");
			break;
		case MPD_STATE_PLAY:
			mpd_status_free(status);
			mpd_connection_free(conn);
			return snprintf(buffer+index,10,"\uf04b");
			break;
	}
	mpd_status_free(status);
	mpd_connection_free(conn);
	return 0;
}


int MPD_get_status_and_song(char buffer[], int index) {
	struct mpd_connection *conn = mpd_connection_new(m_server,m_port,0);

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
			mpd_connection_free(conn);
			return snprintf(buffer+index,10,"\ue099");
			break;
		case MPD_STATE_STOP:
			mpd_status_free(status);
			mpd_connection_free(conn);
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
	mpd_connection_free(conn);
	return written;
}
