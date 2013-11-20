#include "Music.h"
#include "Floppy.h"
#include "HardDrive.h"

#include "Songs/hello.flb"

Floppy *f1, *f2;
HardDrive *hd1;
Song *song1;
Instrument **orchestra;

void setup() {
    f1 = new Floppy(2, 3, 4);
    f2 = new Floppy(6, 7, 8);
    hd1 = new HardDrive(12);
    orchestra = (Instrument**) malloc(3 * sizeof(Instrument*));
    orchestra[0] = f1;
    orchestra[1] = f2;
    orchestra[2] = hd1;

    song1 = new Song(hello);
}

void loop() {
    //playSong(orchestra, song);
    f1->playTrack(song1->tracks[0]);
    //hd1->playTrack(song1->tracks[0]);
    delay(2000);
}

// TODO
void playSong(Instrument **instruments, Song *song) {

}
