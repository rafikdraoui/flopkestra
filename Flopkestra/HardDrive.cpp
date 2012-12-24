#include "HardDrive.h"

/*
 * The hard drive acts the same way as a regular speaker. The input pin is
 * the signal and the other pin is connected to ground.
 */

HardDrive::HardDrive(byte inputpin) {
    inputPin = inputpin;
    pinMode(inputPin, OUTPUT);
}

void HardDrive::playTone(float freq, uint duration) {
    if (freq == 0) {
        delay(duration);
        return;
    }

    // Delay (in µs) = (1 / freq) * 1000000 µs/s
    long del = 1000000 / freq;
    long now = millis();
    while (millis() - now < duration) {
        digitalWrite(inputPin, HIGH);
        delayMicroseconds(del);
        digitalWrite(inputPin, LOW);
        delayMicroseconds(del);
    }
}
