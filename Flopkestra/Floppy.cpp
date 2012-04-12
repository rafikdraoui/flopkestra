#include "Floppy.h"

/*
 *  The DriveSelect pin needs to be pulled LOW for the floppy drive to be
 *  enabled. The motor moves one step in the direction given by the state
 *  of the DIR pin whenever the STEP pin state changes.
 */


Floppy::Floppy(byte driveselectpin, byte steppin, byte dirpin) {
    driveSelectPin = driveselectpin;
    dirPin = dirpin;
    stepPin = steppin;
    pinMode(driveSelectPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
}

void Floppy::playTone(float freq, int duration) {
    if (freq == 0) {
        delay(duration);
        return;
    }

    //Delay (in µs) = (1 / freq / 2) * 1000000 µs/s
    long del = 500000 / freq;

    digitalWrite(driveSelectPin, LOW);
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

