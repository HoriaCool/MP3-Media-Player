// Copyright 2020 Nedelcu Horia (nedelcu.horia.alexandru@gmail.com)

#ifndef _MP3_TRACK_H_
#define _MP3_TRACK_H_

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
   char tag[3];
   char title[30];
   char artist[30];
   char album[30];
   char year[4];
   char comment[30];
   unsigned char genre;
} mp3_tag_t;

typedef struct {
    mp3_tag_t tag;      // track metadata
    char filename[100]; // file name
} mp3_track_t;


void set_mp3filename(mp3_track_t *track, const char *filename) {
    strcpy(track->filename, filename);
}

int print_mp3tag(const mp3_track_t *track, FILE *fp) {
    // Make sure we print what we expect.
    if (memcmp(track->tag.tag, "TAG", 3) != 0) {
        perror("Failed to find TAG");
        return EXIT_FAILURE;
    } else {
        // Found the tag where we expected
        fprintf(fp, "Title: %.30s\n", track->tag.title);
        fprintf(fp, "Artist: %.30s\n", track->tag.artist);
        fprintf(fp, "Album: %.30s\n", track->tag.album);
        fprintf(fp, "Year: %.4s\n", track->tag.year);
         
        if (track->tag.comment[28] == '\0') {
            fprintf(fp, "Comment: %.28s\n", track->tag.comment);
            fprintf(fp, "Track: %d\n", track->tag.comment[29]);
        }
        else {
            fprintf(fp, "Comment: %.30s\n", track->tag.comment);
        }
        
        fprintf(fp, "Genre: %d\n", track->tag.genre);
    }

    return EXIT_SUCCESS;
}

int read_mp3tag(mp3_track_t *track) {
    FILE *fp = fopen(track->filename, "rb");
    if (!fp) {
        perror("File open failed");
        return EXIT_FAILURE;
    }

    // Seek to 128 bytes before the end of the file
    if (fseek(fp, -1 * sizeof(mp3_tag_t), SEEK_END) == -1) {
        perror("fseek failed");
        return EXIT_FAILURE;
    }

    // Read the tag
    if (fread(&track->tag, sizeof(mp3_tag_t), 1, fp) != 1) {
        perror("Failed reading tag");
        return EXIT_FAILURE;
    }

    // Make sure we've got what we expect.
    if (memcmp(track->tag.tag, "TAG", 3) != 0) {
        perror("Failed to find TAG");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int list_mp3files(int *size, char *files[], const char *dirname) { 
    *size = 0;
    char *point;
    struct dirent *de;  // Pointer for directory entry

    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(dirname);

    if (dr == NULL) { // opendir returns NULL if couldn't open directory 
        perror("File open directory");
        return EXIT_FAILURE;
    }

    while ((de = readdir(dr)) != NULL) {
        if ((point = strrchr(de->d_name, '.')) != NULL) {
            if (strcmp(point, ".mp3") == 0) { // ends with mp3
                // +1 for null character '\0' at the end
                files[*size] = malloc(strlen(dirname) + strlen(de->d_name) + 1);
                strcpy(files[*size], dirname);
                strcpy(files[(*size)++] + strlen(dirname), de->d_name);
            }
        }
    }

    closedir(dr);     
    return EXIT_SUCCESS; 
}

#endif /* _MP3_TRACK_H_ */
