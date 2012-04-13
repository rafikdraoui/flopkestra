#ifndef MIDI_TO_FLOPPY_H
#define MIDI_TO_FLOPPY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*** GLOBAL VARIABLES ***/
uint8_t runningStatus;
uint8_t channels[16];


/*** DATA STRUCTURES ***/

typedef enum {NOTE_ON, NOTE_OFF, END_OF_TRACK, UNSUPPORTED} eventKind;

typedef struct EVENT {
    eventKind kind;
    int tick;
    int note;
    int channel;
    struct EVENT *next;
} EVENT;


typedef struct TRACK {
    EVENT *events;
} TRACK;


typedef struct MIDI {
    int format;
    int ntrks;
    int division;
    int channels[16];
    TRACK **tracks;
} MIDI;


/*** FUNCTION PROTOTYPES ***/

MIDI *readMidi(FILE *midifile);
TRACK *readTrack(FILE *midifile);
EVENT *readEvent(FILE *midifile);
EVENT *readMetaEvent(FILE *midifile, int tick, uint8_t head);
EVENT *readSysExEvent(FILE *midifile, int tick);
int readVarLen(FILE *midifile);
MIDI *splitByChannels(MIDI *input);
TRACK *gatherTrack(MIDI *input, int oldChannel, int newChannel);
void absoluteTimesInTicks(MIDI *input);
void writeFloppySong(MIDI *midi, char *outfile);
void FLPtoFLB(char *infile, int songLength, uint16_t *trackLengths);
void handleError(char *msg);


/*** PRETTY PRINTING ***/

void pprintEVENT(EVENT *event) {
    if (event == NULL) return;
    switch(event->kind) {
        case NOTE_ON:
            printf("NOTE ON %d Ch%d at tick %d\n",
                    event->note, event->channel, event->tick);
            break;
        case NOTE_OFF:
            printf("NOTE OFF %d Ch%d at tick %d\n",
                    event->note, event->channel, event->tick);
            break;
        case END_OF_TRACK:
            printf("END_OF_TRACK at tick %d\n", event->tick);
            break;
        case UNSUPPORTED:
            printf("UNSUPPORTED\n");
            break;
    }
}

void pprintTRACK(TRACK *track) {
    if (track == NULL) return;

    EVENT *current = track->events;
    while (1) {
        printf("\t");
        pprintEVENT(current);
        if (current->kind == END_OF_TRACK)
            break;
        current = current->next;
    }
}

void pprintMIDI(MIDI *midi) {
    if (midi == NULL) return;

    int i;
    printf("*** Midi File ***\n");
    printf("Format: %d\n", midi->format);
    printf("Number of tracks: %d\n", midi->ntrks);
    printf("Division: %d\n", midi->division);
    printf("Channel(s) used: ");
    for (i=0; i < 16; i++) {
        if (midi->channels[i])
            printf("%d ", i);
    }
    printf("\n");
    for (i = 0; i < midi->ntrks; i++) {
        printf("Track %d:\n", i);
        pprintTRACK(midi->tracks[i]);
    }
    printf("*****************\n");
}

#endif
