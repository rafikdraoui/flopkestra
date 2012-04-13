#include "Music.h"
#include "Floppy.h"

//#include "Songs/hello.flb"

Floppy *f1, *f2;
Song *song1;
Instrument **orchestra;

void setup() {
    f1 = new Floppy(3, 4, 5);
    f2 = new Floppy(7, 8, 9);
    orchestra = (Instrument**) malloc(2 * sizeof(Instrument*));
    orchestra[0] = f1;
    orchestra[1] = f2;

    //song1 = new Song(hello);
}

void loop() {

    //playSong(orchestra, song);

    //f1->playTrack(song1->tracks[0]);
    delay(2000);
}

// TODO
void playSong(Instrument **instruments, Song *song) {

}
