#include "Floppy.h"

/*
 *  The DriveSelect pin needs to be pulled LOW for the floppy drive to be
 *  enabled. The motor moves one step in the direction given by the state
 *  of the DIR pin whenever the STEP pin state changes.
 */


Floppy::Floppy(byte drvs, byte step, byte dir) {
    driveSelectPin = drvs;
    stepPin = step;
    dirPin = dir;
    pinMode(driveSelectPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
}

void Floppy::playTone(float freq, uint duration) {
    if (freq == 0) {
        delay(duration);
        return;
    }

    digitalWrite(driveSelectPin, LOW);

    // Delay (in µs) = (1 / freq / 2) * 1000000 µs/s
    long del = 500000 / freq;
    long now = millis();
    while (millis() - now < duration) {
        digitalWrite(dirPin, LOW);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(del);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(del);

        digitalWrite(dirPin, HIGH);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(del);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(del);
    }

    digitalWrite(driveSelectPin, LOW);
}

