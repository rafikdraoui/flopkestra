#ifndef SONG_H
#define SONG_H

#include <Arduino.h>
#include "Music.h"

class Track {
public:
    Track(int length, float *notes, int *durations);
    int length;
    float *notes;
    int *durations;
};

class Song {
public:
    Song(char *filename, bool binary);
    int numTracks;
    Track **tracks;
};

#endif
