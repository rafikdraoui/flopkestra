#include "midi_to_floppy.h"


/* if defined, do not remove intermediary .flp file */
//#define DEBUG_FLB


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s midifile\n", argv[0]);
        exit(1);
    }

    char *infile = argv[1];
    FILE *fp;
    if ((fp = fopen(infile, "rb")) == NULL) {
        printf("Error opening MIDI file: %s\n", infile);
        exit(1);
    }

    MIDI *midi = readMidi(fp);
    fclose(fp);

    midi = splitByChannels(midi);
    absoluteTimesInTicks(midi);

    // get song name
    char *title;
    char *basename = strrchr(infile, '/');
    if (basename != NULL)
        title = strdup(basename + 1);
    else
        title = strdup(infile);
    char *c = strrchr(title, '.');
    if (c != NULL) *c = '\0';

    writeFloppySong(midi, title);

    return 0;
}


MIDI *readMidi(FILE *midifile) {
    int i;

    MIDI *midi = (MIDI*) malloc(sizeof(MIDI));
    for (i = 0; i < 16; i++)
        channels[i] = 0;

    char magic[4];
    fread(magic, 1, 4, midifile);
    if ( strncmp(magic, "MThd", 4) != 0 ) {
        handleError("Input file is not a MIDI file");
    }

    int header_length = getc(midifile);
    for (i = 0; i < 3; i++) {
        header_length = (header_length << 8) + getc(midifile);
    }

    midi->format = getc(midifile);
    midi->format = (midi->format << 8) + getc(midifile);

    midi->ntrks = getc(midifile);
    midi->ntrks = (midi->ntrks << 8) + getc(midifile);

    char c = getc(midifile);
    if ( c & 0x8 ) {
        handleError("Time-code based division is not supported");
    }
    midi->division = (c << 8) + getc(midifile);

    // Ignore the rest of the header
    i = header_length - 6;
    while (i--) getc(midifile);

    midi->tracks = (TRACK**) malloc(sizeof(TRACK*) * midi->ntrks);
    for (i = 0; i < midi->ntrks; i++) {
        midi->tracks[i] = readTrack(midifile);
    }

    memcpy(midi->channels, channels, 16);
    return midi;
}


TRACK *readTrack(FILE *midifile) {
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
        firstEvent = readEvent(midifile);
    } while (firstEvent->kind == UNSUPPORTED);

    EVENT *nextEvent,  *currentEvent = firstEvent;
    while(currentEvent->kind != END_OF_TRACK) {
        do {
            nextEvent = readEvent(midifile);
        } while (nextEvent->kind == UNSUPPORTED);

        currentEvent->next = nextEvent;
        currentEvent = nextEvent;
    }
    track->events = firstEvent;
    return track;
}


EVENT *readEvent(FILE *midifile) {
    EVENT *event;
    int tick = readVarLen(midifile);
    uint8_t cmd, channel, data1;
    uint8_t head = getc(midifile);
    char *errorMessage;

    if (head == 0xF0 || head == 0xF7) { // system exclusive event
        runningStatus = 0;
        event = readSysExEvent(midifile, tick);
    }
    else if (head >= 0xF8) { // meta-event a.k.a. system real-time event
        event = readMetaEvent(midifile, tick, head);
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


EVENT *readMetaEvent(FILE *midifile, int tick, uint8_t head) {
    EVENT *event = (EVENT*) malloc(sizeof(EVENT));
    uint8_t type = getc(midifile);
    int i, length = readVarLen(midifile);
    for (i = 0; i < length; i++)
        getc(midifile);
    if (head == 0xFF && type == 0x2F)
        event->kind = END_OF_TRACK;
    else //TODO: handle FF 58 (set time sig) and FF 51 (set tempo)
        event->kind = UNSUPPORTED;
    event->tick = tick;
    return event;
}


EVENT *readSysExEvent(FILE *midifile, int tick) {
    EVENT *event = (EVENT*) malloc(sizeof(EVENT));
    int i, length = readVarLen(midifile);
    for (i = 0; i < length; i++)
        getc(midifile);
    event->tick = tick;
    event->kind = UNSUPPORTED;
    return event;
}


int readVarLen(FILE *midifile) {
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
    uint8_t realChannels[16];
    for (i = 0; i < 16; i++) {
        if (input->channels[i]) {
            numChannels++;
            realChannels[j++] = i;
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
        output->tracks[i] = gatherTrack(input, realChannels[i], i);
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


void writeFloppySong(MIDI *midi, char *title) {
    int i, state, note;
    int time_on, duration;
    uint16_t trackLength, songLength = 0;
    uint16_t trackLengths[midi->ntrks];

    char *tmpFileName = (char*) malloc(strlen(title) + 5);
    strcpy(tmpFileName, title);
    strcat(tmpFileName, ".flp");
    FILE *tmp;
    if ((tmp = fopen(tmpFileName, "w+")) == NULL)
        handleError("Cannot create output .flp file");

    fprintf(tmp, "TRACKS: %d\n", midi->ntrks);

    EVENT *event;
    for (i = 0; i < midi->ntrks; i++) {
        trackLength = 0;
        fprintf(tmp, "INSTRUMENT %d\n", i);
        event = midi->tracks[i]->events;
        state = 0;
        note = -1;
        time_on = 0;
        while (event->kind != END_OF_TRACK) {
            if (state == 0) {   // no note in progess
                if (event->kind == NOTE_ON) {
                    duration = (event->tick - time_on) * MS_PER_TICK;
                    if (duration > 0) {
                        // silence (that has just been ended by NOTE_ON)
                        fprintf(tmp, "0 %d\n", duration);
                        trackLength++;
                    }

                    state = 1;
                    note = event->note;
                    time_on = event->tick;
                }
            }
            else {  // a note is in progress
                if (event->kind == NOTE_OFF && event->note == note) {
                    duration = (event->tick - time_on) * MS_PER_TICK;
                    fprintf(tmp, "%d %d\n", note, duration);
                    trackLength++;
                    state = 0;
                    note = -1;
                    time_on = event->tick;  // beginning of silence
                }
                //TODO: do not drop other notes
            }
            event = event->next;
        }
        songLength += trackLength;
        fprintf(tmp, "END %d (LENGTH %d)\n", i, trackLength);
        trackLengths[i] = (uint16_t) trackLength;
    }
    fclose(tmp);

    FLPtoFLB(tmpFileName, songLength, trackLengths);
}


/*
 * FLB = song_length numtracks (TRACK)+
 * TRACK = track_length (TONE)+
 * TONE = note duration
 *
 * song_length, track_length and duration are 2 bytes long,
 * numtracks and note are 1 byte long.
 */
#define APPEND(byte) (songArray[arrayIndex++] = (uint8_t) (byte))
void FLPtoFLB(char *infile, int songLength, uint16_t *trackLengths) {

    int i, j, dummy;
    int ntrks, bnote, bduration;
    int arrayLength, arrayIndex = 0;

    FILE *in;
    if ((in = fopen(infile, "r")) == NULL)
        handleError("Cannot open flp file");

    fscanf(in, "TRACKS: %d\n", &ntrks);
    arrayLength = (songLength * 3) + (2 * ntrks) + 3;

    uint8_t songArray[arrayLength];

    APPEND(arrayLength >> 8);
    APPEND(arrayLength);
    APPEND(ntrks);

    for (i = 0; i < ntrks; i++) {
        fscanf(in, "INSTRUMENT %d\n", &dummy);
        APPEND(trackLengths[i] >> 8);
        APPEND(trackLengths[i]);
        for (j = 0; j < trackLengths[i]; j++) {
            fscanf(in, "%d %d\n", &bnote, &bduration);
            APPEND(bnote);
            APPEND(bduration >> 8);
            APPEND(bduration);
        }
        fscanf(in, "END %d (LENGTH %d)\n", &dummy, &dummy);
    }
    fclose(in);

    char *outfile = strdup(infile);
    char *arrayName = strdup(infile);
    outfile[strlen(infile)-1] = 'b';
    arrayName[strlen(infile)-4] = '\0';

    FILE *out;
    if ((out = fopen(outfile, "w")) == NULL)
        handleError("Cannot create flb file");

    fprintf(out, "const byte %s[] PROGMEM = {\n", arrayName);
    for (i = 0; i < arrayLength; i++) {
        fprintf(out, "0x%X", songArray[i]);
        if (i < (arrayLength - 1))
            fprintf(out, ", ");
    }
    fprintf(out, "\n};\n");
    fclose(out);

    #ifndef DEBUG_FLB
    remove(infile);
    #endif
}


void handleError(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}
