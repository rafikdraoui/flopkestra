#include "Song.h"

#define getNextByte(song) pgm_read_byte(song + songIndex++)


Track::Track(int l, float *n, uint *d) {
    length = l;
    notes = n;
    durations = d;
}

/* songName is the name of a byte array in PROGMEM containing the bytecode
   for the song */
Song::Song(const byte *songName) {

    int songIndex = 0;
    int songLength, trackLength;

    int numTracks, note;
    uint duration;
    float *notes;
    uint *durations;

    songLength = getNextByte(songName);
    songLength = (songLength << 8) + getNextByte(songName);
    numTracks = getNextByte(songName);
    tracks = (Track**) malloc(numTracks * sizeof(Track*));
    for (int i = 0; i < numTracks; i++) {
        trackLength = getNextByte(songName);
        trackLength = (trackLength << 8) + getNextByte(songName);

        notes = (float*) malloc(trackLength * sizeof(float));
        durations = (uint*) malloc(trackLength * sizeof(uint));
        for (int j = 0; j < trackLength; j++) {
            note = getNextByte(songName);
            duration = getNextByte(songName);
            duration = (duration << 8) + getNextByte(songName);

            notes[j] = MIDI_TO_FREQ[note];
            durations[j] = duration;
        }
        tracks[i] = new Track(trackLength, notes, durations);
    }
}

