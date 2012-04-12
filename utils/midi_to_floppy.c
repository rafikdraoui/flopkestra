#include "midi_to_floppy.h"

#define MS_PER_TICK 1   //FIXME


/*** GLOBAL VARIABLES ***/
FILE *midifile;
int runningStatus;
int channels[16];


int main(int argc, char *argv[]) {

    int binary_opt;
    char *infile, *outfile;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s [-b] midifile\n", argv[0]);
        exit(1);
    }

    if (argc == 3 && strncmp(argv[1], "-b", 2) == 0) {
        binary_opt = 1;
        infile = argv[2];
    }
    else {
        binary_opt = 0;
        infile = argv[1];
    }

    if ((midifile = fopen(infile, "rb")) == NULL) {
        printf("Error opening MIDI file: %s\n", infile);
        exit(1);
    }

    MIDI *midi = readMidi();
    midi = splitByChannels(midi);
    absoluteTimesInTicks(midi);

    // create output file name (file.mid --> file.fl{b,p})
    outfile = (char*) malloc(strlen(infile) + 5);
    outfile = strdup(infile);
    char *c = strchr(outfile, '.');
    if (c == NULL)
        strcat(outfile, (binary_opt ? ".flb" : ".flp"));
    else
        strcpy(c, (binary_opt ? ".flb" : ".flp"));

    writeFloppySong(midi, binary_opt, outfile);

    fclose(midifile);
    return 0;
}


//TODO: handle binary format
void writeFloppySong(MIDI *midi, int binary_opt, char *outfile) {
    int i, state, note, time_on, duration;
    EVENT *event;
    FILE *fp = fopen(outfile, "w");
    fprintf(fp, "FLOP %d\n", midi->ntrks);

    for (i = 0; i < midi->ntrks; i++) {
        fprintf(fp, "INSTRUMENT %d\n", i);
        event = midi->tracks[i]->events;
        state = 0;
        note = -1;
        while (event->kind != END_OF_TRACK) {
            if (state == 0) {   // no note in progess
                if (event->kind == NOTE_ON) {
                    state = 1;
                    note = event->note;
                    time_on = event->tick;
                }
            }
            else {  // a note is in progress
                if (event->kind == NOTE_OFF && event->note == note) {
                    duration = (event->tick - time_on) * MS_PER_TICK;
                    fprintf(fp, "%d %d\n", note, duration);
                    state = 0;
                    note = -1;
                } //TODO: do not drop other notes
            }
            event = event->next;
        }
        fprintf(fp, "END %d\n", i);
    }
    fclose(fp);
}



MIDI *readMidi(void) {
    int i;
    char c;

    MIDI *midi = (MIDI*) malloc(sizeof(MIDI));
    for (i = 0; i < 16; i++)
        channels[i] = 0;

    char magic[4];
    fread(magic, 1, 4, midifile);
    if ( strncmp(magic, "MThd", 4) != 0 ) {
        handleError("Input file is not a MIDI file");
    }

    int length = getc(midifile);
    for (i = 0; i < 3; i++) {
        length = (length << 8) + getc(midifile);
    }

    midi->format = getc(midifile);
    midi->format = (midi->format << 8) + getc(midifile);

    midi->ntrks = getc(midifile);
    midi->ntrks = (midi->ntrks << 8) + getc(midifile);

    c = getc(midifile);
    if ( c & 0x8 ) {
        handleError("Time-code based division is not supported");
    }
    midi->division = (c << 8) + getc(midifile);

    // Ignore the rest of the header
    i = length - 6;
    while (i--) getc(midifile);

    midi->tracks = (TRACK**) malloc(sizeof(TRACK*) * midi->ntrks);
    for (i = 0; i < midi->ntrks; i++) {
        midi->tracks[i] = readTrack();
    }

    memcpy(midi->channels, channels, 16);
    return midi;
}


TRACK *readTrack(void) {
    runningStatus = 0;
    TRACK *track = (TRACK*) malloc(sizeof(TRACK));

    char magic[4];
    fread(magic, 1, 4, midifile);
    if ( strncmp(magic, "MTrk", 4) != 0 ) {
        handleError("Malformed track header");
    }

    int i, length = getc(midifile);
    for (i = 0; i < 3; i++) {
        length = (length << 8) + getc(midifile);
    }

    EVENT *firstEvent;
    do {
        firstEvent = readEvent();
    } while (firstEvent->kind == UNSUPPORTED);

    EVENT *nextEvent,  *currentEvent = firstEvent;
    while(currentEvent->kind != END_OF_TRACK) {
        do {
            nextEvent = readEvent();
        } while (nextEvent->kind == UNSUPPORTED);

        currentEvent->next = nextEvent;
        currentEvent = nextEvent;
    }
    track->events = firstEvent;
    return track;
}


EVENT *readEvent(void) {
    EVENT *event;
    int tick = readVarLen();
    int cmd, channel, data1;
    int head = getc(midifile);
    char *errorMessage;

    if (head == 0xF0 || head == 0xF7) { // system exclusive event
        runningStatus = 0;
        event = readSysExEvent(tick);
    }
    else if (head >= 0xF8) { // meta-event a.k.a. system real-time event
        event = readMetaEvent(tick, head);
    }
    else {
        if (head >= 0x80) { // status byte
            runningStatus = head;
            data1 = getc(midifile); // first data byte
        }
        else {
            data1 = head; // first data byte already read
        }
        cmd = (runningStatus >> 4) & 0x0F;
        channel = runningStatus & 0x0F;

        event = (EVENT*) malloc(sizeof(EVENT));
        switch (cmd) {
            case 0x8: // note off
                event->kind = NOTE_OFF;
                event->tick = tick;
                event->channel = channel;
                event->note = data1;
                getc(midifile); // velocity is ignored
                channels[channel] = 1;
                break;
            case 0x9: // note on
                event->tick = tick;
                event->channel = channel;
                event->note = data1;
                if (getc(midifile))
                    event->kind = NOTE_ON;
                else  // velocity of 0 corresponds to a NOTE OFF
                    event->kind = NOTE_OFF;
                channels[channel] = 1;
                break;
            case 0xA:
            case 0xB:
            case 0xE:
                getc(midifile); // second data byte (ignored)
            case 0xC:
            case 0xD:
                event->kind = UNSUPPORTED;
                break;
            default:
                errorMessage = (char*) malloc(24);
                sprintf(errorMessage, "Unrecognized event: %X", head);
                handleError(errorMessage);
        }
    }

    return event;
}


EVENT *readMetaEvent(int tick, int head) {
    EVENT *event = (EVENT*) malloc(sizeof(EVENT));
    int type = getc(midifile);
    int i, length = readVarLen();
    for (i = 0; i < length; i++)
        getc(midifile);
    if (head == 0xFF && type == 0x2F)
        event->kind = END_OF_TRACK;
    else //TODO: handle FF 58 (set time sig) and FF 51 (set tempo)
        event->kind = UNSUPPORTED;
    event->tick = tick;
    return event;
}


EVENT *readSysExEvent(int tick) {
    EVENT *event = (EVENT*) malloc(sizeof(EVENT));
    int i, length = readVarLen();
    for (i = 0; i < length; i++)
        getc(midifile);
    event->tick = tick;
    event->kind = UNSUPPORTED;
    return event;
}


int readVarLen(void) {
    int value = 0;
    char c;
    if ( (value = getc(midifile)) & 0x80 ) {
        value &= 0x7f;
        do {
            value = (value << 7) + ((c = getc(midifile)) & 0x7f);
        } while ( c & 0x80 );
    }
    return value;
}


MIDI *splitByChannels(MIDI *input) {

    int i, j = 0, numChannels = 0;
    int channels[16];
    for (i = 0; i < 16; i++) {
        if (input->channels[i]) {
            numChannels++;
            channels[j++] = i;
        }
    }

    MIDI *output = (MIDI*) malloc(sizeof(MIDI));
    output->format = 1;
    output->ntrks = numChannels;
    output->division = input->division;
    for (i = 0; i < numChannels; i++)
        output->channels[i] = 1;

    output->tracks = (TRACK**) malloc(sizeof(TRACK*) * numChannels);
    for (i = 0; i < numChannels; i++) {
        output->tracks[i] = gatherTrack(input, channels[i], i);
    }

    return output;
}


TRACK *gatherTrack(MIDI *input, int oldChannel, int newChannel) {

    int i, time;
    EVENT *currentEvent, *newEvent, *firstEvent, *prevEvent;
    firstEvent = prevEvent = NULL;
    TRACK *outputTrack, *currentTrack;
    outputTrack = (TRACK*) malloc(sizeof(TRACK));

    for (i = 0; i < input->ntrks; i++) {
        time = 0;
        currentTrack = input->tracks[i];
        currentEvent = currentTrack->events;
        while (currentEvent->kind != END_OF_TRACK) {
            time += currentEvent->tick;
            if (currentEvent->channel == oldChannel) {
                newEvent = (EVENT*) malloc(sizeof(EVENT));
                newEvent->kind = currentEvent->kind;
                newEvent->tick = time;
                newEvent->note = currentEvent->note;
                newEvent->channel = newChannel;
                if (prevEvent != NULL)
                    prevEvent->next = newEvent;
                if (firstEvent == NULL)
                    firstEvent = newEvent;
                prevEvent = newEvent;
                time = 0;
            }
            currentEvent = currentEvent->next;
        }
    }

    EVENT *endOfTrack = (EVENT*) malloc(sizeof(EVENT));
    endOfTrack->kind = END_OF_TRACK;
    endOfTrack->tick = 0;
    prevEvent->next = endOfTrack;

    outputTrack->events = firstEvent;
    return outputTrack;
}


void absoluteTimesInTicks(MIDI *midi) {
    int i, time;
    EVENT *currentEvent;
    for (i = 0; i < midi->ntrks; i++) {
        time = 0;
        currentEvent = midi->tracks[i]->events;
        while (1) {
            time += currentEvent->tick;
            currentEvent->tick = time;
            if (currentEvent->kind == END_OF_TRACK)
                break;
            currentEvent = currentEvent->next;
        }
    }
}


void handleError(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}
