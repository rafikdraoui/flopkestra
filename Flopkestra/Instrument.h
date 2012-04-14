#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <Arduino.h>
#include "Song.h"

class Instrument {
public:
    virtual void playTone(float frequency, uint duration) = 0;

    void playTrack(Track *track) {
        for (int i = 0; i < track->length; i++) {
            playTone(track->notes[i], track->durations[i]);
        }
    };
};


#endif
