#include "Floppy.h"

Floppy *f1;
Song *song;

void setup() {
    f1 = new Floppy(3, 4, 5);
    song = new Song(NULL, false);
}

void loop() {

    /*
    for (int i = 0; i < 6 * 12; i++) {
        f1->playTone(NOTES[i], 500);
    }
    */

    f1->playTrack(song->tracks[0]);

    delay(2000);
}
