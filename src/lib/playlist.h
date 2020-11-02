// Copyright 2020 Nedelcu Horia (nedelcu.horia.alexandru@gmail.com)

#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_

#include "mp3_track.h"
#include "audio_driver.h"

struct playlist_item {
    mp3_track_t track;
    struct playlist_item *next, *previous;
};

struct playlist_item* create_item(const mp3_track_t track) {
    struct playlist_item *node = NULL;

    node = (struct playlist_item*) malloc(sizeof(struct playlist_item));
    node->track = track;
    node->previous = NULL;
    node->next = NULL;

    return node;
}


typedef struct {
    int item_count;
    struct playlist_item *head, *tail, *current_track;
    audio_driver_t audio;
} playlist_t;

playlist_t new_playlist() {
    playlist_t playlist;

    playlist.item_count = 0;
    playlist.head = NULL;
    playlist.tail = NULL;
    playlist.current_track = NULL;

    initialize_audio(&playlist.audio);

    return playlist;
}


void next_track(playlist_t *playlist) {
	if (playlist->current_track == NULL) {
        perror("No track playing"); 
    } else {
    	if (playlist->current_track->next != NULL) {
    		playlist->current_track = playlist->current_track->next;

        	get_decoding_format(&playlist->audio,
                playlist->current_track->track.filename);
    	}
    }
}

void prev_track(playlist_t *playlist) {
	if (playlist->current_track == NULL) {
        perror("No track playing"); 
    } else {
    	if (playlist->current_track->previous != NULL) {
    		playlist->current_track = playlist->current_track->previous;
    	}

    	get_decoding_format(&playlist->audio,
            playlist->current_track->track.filename);
    }
}


void add_front(playlist_t *playlist, const mp3_track_t track) {
    struct playlist_item *node = create_item(track);

    if (playlist->item_count) {
        node->next = playlist->head;
        playlist->head->previous = node;
        playlist->head = node;
    } else {
        playlist->head = node;
        playlist->tail = node;
    }

    playlist->item_count++;

    if (playlist->current_track == NULL) {
        playlist->current_track = playlist->head;
        get_decoding_format(&playlist->audio,
            playlist->current_track->track.filename);
    }
}

void add_end(playlist_t *playlist, const mp3_track_t track) {
    struct playlist_item *node = create_item(track);

    if (playlist->item_count) {
        node->previous = playlist->tail;
        playlist->tail->next = node;
        playlist->tail = node;
    } else {
        playlist->head = node;
        playlist->tail = node;
    }

    playlist->item_count++;

    if (playlist->current_track == NULL) {
        playlist->current_track = playlist->head;
        get_decoding_format(&playlist->audio,
            playlist->current_track->track.filename);
    }
}

void add_after(playlist_t *playlist, const mp3_track_t track) {
    struct playlist_item *node = create_item(track);

    if (playlist->current_track == NULL) {
        // perror("No track playing");
        free(node);
        add_end(playlist, track);
        return;
    }

    if (playlist->current_track == playlist->tail) {
        free(node);
        add_end(playlist, track);
    } else {
        node->next = playlist->current_track->next;
        node->previous = playlist->current_track;

        playlist->current_track->next->previous = node;
        playlist->current_track->next = node;

        playlist->item_count++;
    }
}


void del_front(playlist_t *playlist) {
    struct playlist_item *node = playlist->head;

    if (playlist->item_count) {
    	if (playlist->current_track == playlist->head) {
    	    playlist->current_track = playlist->head->next;

        	if (playlist->current_track != NULL) {
        		get_decoding_format(&playlist->audio,
                    playlist->current_track->track.filename);
        	}
    	}

        playlist->head = playlist->head->next;
        free(node);

        if (playlist->head != NULL) { 
	        playlist->head->previous = NULL;
	    } else {
	    	playlist->tail = NULL;
	    }

        playlist->item_count--;
    }
}

void del_end(playlist_t *playlist) {
    struct playlist_item *node = playlist->tail;

    if (playlist->item_count) {
    	if (playlist->current_track == playlist->tail) {
    	    playlist->current_track = playlist->tail->previous;

            if (playlist->current_track != NULL) {
        		get_decoding_format(&playlist->audio,
                    playlist->current_track->track.filename);
        	}
    	}

        playlist->tail = playlist->tail->previous;
        free(node);

        if (playlist->tail != NULL) { 
	        playlist->tail->next = NULL;
	    } else {
	    	playlist->head = NULL;
	    }

        playlist->item_count--;
    }
}

void del_curr(playlist_t *playlist) {
	struct playlist_item *node = playlist->current_track;

    if (playlist->current_track == NULL) {
        perror("No song to delete");
    } else {
    	if (playlist->current_track == playlist->head) {
    		del_front(playlist);	
    	} else if (playlist->current_track == playlist->tail) {
	    	del_end(playlist);
	    } else {
	    	node->next->previous = node->previous;
    		node->previous->next = node->next;
    		
    		playlist->current_track = node->next;

    		if (playlist->current_track != NULL) {
        		get_decoding_format(&playlist->audio,
                    playlist->current_track->track.filename);
        	}

    		free(node);

    		playlist->item_count--;
    	}
    }
}


void start_playlist(playlist_t *playlist) {
    if (playlist->item_count) {
        playlist->current_track = playlist->head;
        get_decoding_format(&playlist->audio,
            playlist->head->track.filename);
    } else {
        perror("No song to play");
    }
}

void decode_and_play_current_track(playlist_t *playlist) {
    if (playlist->current_track != NULL) {
        if (!decode_and_play(&playlist->audio)) {
            /* jump to next track */
            playlist->current_track = playlist->current_track->next;
        
            /* check if is not the end of the playlist */
            if (playlist->current_track != NULL) {
                get_decoding_format(&playlist->audio,
                    playlist->current_track->track.filename);
            }
        }
    }
}

#endif /* _PLAYLIST_H_ */
