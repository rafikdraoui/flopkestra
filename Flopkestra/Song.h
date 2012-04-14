#ifndef SONG_H
#define SONG_H

#include <Arduino.h>
#include "Music.h"

typedef unsigned int    uint;

class Track {
public:
    Track(int length, float *notes, uint *durations);
    int length;
    float *notes;
    uint *durations;
};

class Song {
public:
    Song(const byte *songName);
    int numTracks;
    Track **tracks;
};

#endif
