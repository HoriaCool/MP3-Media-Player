// Copyright 2020 Nedelcu Horia (nedelcu.horia.alexandru@gmail.com)

#ifndef _AUDIO_DRIVER_H_
#define _AUDIO_DRIVER_H_

#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8

typedef struct {
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;
} audio_driver_t;


void initialize_audio(audio_driver_t *audio) {
    /* initializations */
    ao_initialize();
    audio->driver = ao_default_driver_id();
    mpg123_init();
    audio->mh = mpg123_new(NULL, &audio->err);
    audio->buffer_size = mpg123_outblock(audio->mh);
    audio->buffer = (unsigned char*) malloc(audio->buffer_size * sizeof(unsigned char));
}

void get_decoding_format(audio_driver_t *audio, const char *mp3filename) {
    /* open the file and get the decoding format */
    mpg123_open(audio->mh, mp3filename);
    mpg123_getformat(audio->mh, &audio->rate, &audio->channels, &audio->encoding);

    /* set the output format and open the output device */
    audio->format.bits = mpg123_encsize(audio->encoding) * BITS;
    audio->format.rate = audio->rate;
    audio->format.channels = audio->channels;
    audio->format.byte_format = AO_FMT_NATIVE;
    audio->format.matrix = 0;
    audio->dev = ao_open_live(audio->driver, &audio->format, NULL);
}

int decode_and_play(audio_driver_t *audio) {
    if (mpg123_read(audio->mh, audio->buffer, audio->buffer_size, &audio->done) == MPG123_OK) {
        /* still playing this track */
        ao_play(audio->dev, audio->buffer, audio->done);
        return 1;
    } else {
        /* initialize next track */
        return 0;
    }
}

#endif /* _AUDIO_DRIVER_H_ */
