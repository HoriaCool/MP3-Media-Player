// Copyright 2020 Nedelcu Horia (nedelcu.horia.alexandru@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h> 
#include <unistd.h> 
#include <gtk/gtk.h>

#include "./playlist.h"
#include "./mp3_track.h"
#include "./audio_driver.h"

#define SONGS_DIRECTORY_NAME "./songs/"
#define MAX_SONGS_NUMBER 100

#define SPACE_DIM 50
#define BUTTON_WIDTH 80
#define BUTTON_BIG_LENGTH (4 * SPACE_DIM + 5 * BUTTON_WIDTH)
#define BUTTON_SMALL_LENGTH (2 * SPACE_DIM + 3 * BUTTON_WIDTH)
#define WINDOW_WIDTH (5 * SPACE_DIM + 4 * BUTTON_WIDTH)
#define WINDOW_LENGTH (6 * SPACE_DIM + 5 * BUTTON_WIDTH)

const char *LABEL_FORMAT = "<span foreground='#00FF00' font_desc='18' "
						   "style=\"italic\">\%s</span>";
sem_t mutex;
pthread_mutex_t lock;
pthread_t thread_GUI, thread_AUDIO;

int size, index_song;
char *mp3files[MAX_SONGS_NUMBER];
mp3_track_t tracks[MAX_SONGS_NUMBER];
playlist_t playlist;
int pause_playlist;

char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
    return result;
}

static void btn_s_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    pause_playlist = 1;
    start_playlist(&playlist);
    pause_playlist = 0;

    pthread_mutex_unlock(&lock);
}

static void btn_d_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    int tmp = pause_playlist;

    pause_playlist = 1;
    del_curr(&playlist);
    pause_playlist = tmp;

    pthread_mutex_unlock(&lock);
}

static void btn_prev_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    pause_playlist = 1;
    prev_track(&playlist);
    pause_playlist = 0;

    pthread_mutex_unlock(&lock);
}

static void btn_pr_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    pause_playlist = (pause_playlist + 1) % 2;

    pthread_mutex_unlock(&lock);
}

static void btn_next_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    pause_playlist = 1;
    next_track(&playlist);
    pause_playlist = 0;

    pthread_mutex_unlock(&lock);
}

static void btn_mvd_event (GtkWidget *widget, gpointer data) {
    GtkWidget *lbl_songs = (GtkWidget*) data;
    char *text, *markup;

    index_song = (index_song - 1 + size) % size;
    text = concat("File songs: ", tracks[index_song].tag.title);

    markup = g_markup_printf_escaped(LABEL_FORMAT, text);
    gtk_label_set_markup(GTK_LABEL(lbl_songs), markup);

    free(text);
    g_free(markup);
}

static void btn_add_event (GtkWidget *widget, gpointer data) {
    pthread_mutex_lock(&lock);

    int tmp = pause_playlist;

    pause_playlist = 1;
    add_after(&playlist, tracks[index_song]);
    pause_playlist = tmp;

    pthread_mutex_unlock(&lock);
}

static void btn_mvu_event (GtkWidget *widget, gpointer data) {
    GtkWidget *lbl_songs = (GtkWidget*) data;
    char *text, *markup;

    index_song = (index_song + 1) % size;
    text = concat("File songs: ", tracks[index_song].tag.title);
    markup = g_markup_printf_escaped(LABEL_FORMAT, text);
    gtk_label_set_markup(GTK_LABEL(lbl_songs), markup);

    free(text);
    g_free(markup);
}

static gboolean idle_callback_lbl_ctrack(gpointer data) {
	GtkWidget *lbl_ctrack = (GtkWidget*) data;
	char *text, *markup;

	pthread_mutex_lock(&lock);
    if (playlist.current_track != NULL) {
		text = concat("Current track: ",
            playlist.current_track->track.tag.title);
	} else {
		text = concat("Current track: ", "");
	}

	markup = g_markup_printf_escaped(LABEL_FORMAT, text);
    gtk_label_set_markup(GTK_LABEL(lbl_ctrack), markup);

    free(text);
    g_free(markup);

	pthread_mutex_unlock(&lock);

	return TRUE;
}

void* mp3_app_GUI(void *vargp) {
    GtkWidget *window;
    GtkWidget *fixed;

    GtkWidget *btn_s; 			// button start/restart playlist
    GtkWidget *btn_d; 			// button delete from_playlist
    GtkWidget *btn_prev;		// button previous track
    GtkWidget *btn_next;		// button next track
    GtkWidget *btn_pr;			// button pause resume
    GtkWidget *btn_mvd;			// button move down list of songs
    GtkWidget *btn_mvu;			// button move up list of songs
    GtkWidget *btn_add;			// button add song to playlist

    GtkWidget *lbl_ctrack;		// label current track
    GtkWidget *lbl_songs;		// label folder songs

    /*
     * Initialization main window and fixed container
     */
    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "");
    gtk_window_set_default_size(GTK_WINDOW(window),
    	WINDOW_LENGTH, WINDOW_WIDTH);
    gtk_window_set_position(GTK_WINDOW(window),
    	GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    /*
     * Initialization labels
     */
    char *text, *markup;

    lbl_ctrack = gtk_label_new(NULL);
    text = "Current track: ";
    markup = g_markup_printf_escaped(LABEL_FORMAT, text);
    gtk_label_set_markup(GTK_LABEL(lbl_ctrack), markup);
    g_free(markup);
    gtk_fixed_put(GTK_FIXED(fixed), lbl_ctrack, SPACE_DIM, 2 * SPACE_DIM);

    lbl_songs = gtk_label_new(NULL);
    text = concat("File songs: ", tracks[index_song].tag.title);
    markup = g_markup_printf_escaped(LABEL_FORMAT, text);
    gtk_label_set_markup(GTK_LABEL(lbl_songs), markup);
    free(text);
    g_free(markup);
    gtk_fixed_put(GTK_FIXED(fixed), lbl_songs, SPACE_DIM, 6 * SPACE_DIM + 2 * BUTTON_WIDTH);

    /*
     * Initialization buttons
     */
    btn_s = gtk_button_new_with_label("S");
    gtk_fixed_put(GTK_FIXED(fixed), btn_s,
    	SPACE_DIM, 2 * SPACE_DIM + BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_s, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_s, "clicked", G_CALLBACK(btn_s_event), NULL);

    btn_d = gtk_button_new_with_label("D");
    gtk_fixed_put(GTK_FIXED(fixed), btn_d,
    	2 * SPACE_DIM + BUTTON_WIDTH, 2 * SPACE_DIM + BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_d, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_d, "clicked", G_CALLBACK(btn_d_event), NULL);

   	btn_prev = gtk_button_new_with_label("<");
    gtk_fixed_put(GTK_FIXED(fixed), btn_prev,
    	3 * SPACE_DIM + 2 * BUTTON_WIDTH, 2 * SPACE_DIM + BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_prev, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(btn_prev_event), NULL);

    btn_pr = gtk_button_new_with_label("| |");
    gtk_fixed_put(GTK_FIXED(fixed), btn_pr,
    	4 * SPACE_DIM + 3 * BUTTON_WIDTH, 2 * SPACE_DIM + BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_pr, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_pr, "clicked", G_CALLBACK(btn_pr_event), NULL);

    btn_next = gtk_button_new_with_label(">");
    gtk_fixed_put(GTK_FIXED(fixed), btn_next,
    	5 * SPACE_DIM + 4 * BUTTON_WIDTH, 2 * SPACE_DIM + BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_next, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_next, "clicked", G_CALLBACK(btn_next_event), NULL);

    btn_mvd = gtk_button_new_with_label("DOWN");
    gtk_fixed_put(GTK_FIXED(fixed), btn_mvd,
    	SPACE_DIM, 3 * SPACE_DIM + 2 * BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_mvd, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_mvd, "clicked", G_CALLBACK(btn_mvd_event), lbl_songs);

    btn_add = gtk_button_new_with_label("ADD");
    gtk_fixed_put(GTK_FIXED(fixed), btn_add,
    	2 * SPACE_DIM + BUTTON_WIDTH, 3 * SPACE_DIM + 2 * BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_add, 2 * SPACE_DIM + 3 * BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_add, "clicked", G_CALLBACK(btn_add_event), NULL);

    btn_mvu = gtk_button_new_with_label("UP");
    gtk_fixed_put(GTK_FIXED(fixed), btn_mvu,
    	5 * SPACE_DIM + 4 * BUTTON_WIDTH , 3 * SPACE_DIM + 2 * BUTTON_WIDTH);
    gtk_widget_set_size_request(btn_mvu, BUTTON_WIDTH, BUTTON_WIDTH);
    g_signal_connect(btn_mvu, "clicked", G_CALLBACK(btn_mvu_event), lbl_songs);

    g_signal_connect(G_OBJECT(window), "destroy",
    	G_CALLBACK(gtk_main_quit), NULL);

    gdk_threads_add_idle(idle_callback_lbl_ctrack, lbl_ctrack);
    gtk_widget_show_all(window);
    gtk_main();

    pthread_kill(thread_AUDIO, 1);
}

void* mp3_app_AUDIO(void *vargp) {
    char *text, *markup;

    while(1) {
        sem_wait(&mutex);

        if (!pause_playlist) {
            decode_and_play_current_track(&playlist);

            if (playlist.current_track == NULL) {
                playlist.current_track = playlist.head;
                pause_playlist = 1;
            }
        }

        sem_post(&mutex);
    }
}
