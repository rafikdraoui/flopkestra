#include "Song.h"

Track::Track(int l, float *n, int *d) {
    length = l;
    notes = n;
    durations = d;
}

//TODO
Song::Song(char *filename, bool binary) {

    if (filename == NULL) {
        /*
        length = 12;
        notes = (float*) malloc(sizeof(float) * length);
        durations = (int*) malloc(sizeof(int) * length);
        float tmpnotes[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                            369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

        for (int i = 0; i < length; i++) {
            notes[i] = tmpnotes[i];
            durations[i] = 500;
        }
        */

        numTracks = 1;
        int length = 26;
        float *notes = (float*) malloc(length * sizeof(float));
        int *durations = (int*) malloc(length * sizeof(int));
        int midinotes[] = {
            64, 62, 60, 62, 64, 64, 64, 62, 62, 62, 64, 67, 67, 64, 62,
            60, 62, 64, 64, 64, 64, 62, 62, 64, 62, 60
        };
        int times[] = {
            231, 231, 231, 231, 231, 231, 462,
            231, 231, 462, 231, 231, 462,
            231, 231, 231, 231, 231, 231,
            231, 231, 231, 231, 231, 231, 974
        };
        for (int i = 0; i < length; i++) {
            notes[i] = MIDI_TO_FREQ[midinotes[i]];
            durations[i] = times[i];
        }

        tracks = (Track**) malloc(numTracks * sizeof(Track*));
        tracks[0] = new Track(length, notes, durations);
    }
}
