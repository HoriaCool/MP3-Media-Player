 
// Copyright 2020 Nedelcu Horia (nedelcu.horia.alexandru@gmail.com)

#include "./lib/mp3_window.h"

int main(int argc, char *argv[]) {
    playlist = new_playlist();
    pause_playlist = 1;

    list_mp3files(&size, mp3files, SONGS_DIRECTORY_NAME);
    index_song = 0;
    if (size == 0) {
        perror("No song available in songs directory");
        return 0;
    }

    for (int i = 0; i < size; ++i) {
        set_mp3filename(&tracks[i], mp3files[i]);
        read_mp3tag(&tracks[i]);
    }

    for (int i = 0; i < size; ++i) {
        free(mp3files[i]);
    }

    sem_init(&mutex, 0, 1);

    pthread_create(&thread_GUI, NULL, mp3_app_GUI, NULL);
    pthread_create(&thread_AUDIO, NULL, mp3_app_AUDIO, NULL);

    pthread_join(thread_GUI, NULL);
    pthread_join(thread_AUDIO, NULL);

    sem_destroy(&mutex);

    return 0;
}
