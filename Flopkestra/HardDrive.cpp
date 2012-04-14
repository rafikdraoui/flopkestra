#include "HardDrive.h"

/*
 * The hard drive acts the same way as a regular speaker. The input pin is
 * the signal and the other pin is connected to ground.
 */

HardDrive::HardDrive(byte inputpin) {
    inputPin = inputpin;
}

/* Note: using `tone' means that only one hard drive instrument can be
   used at a time. */
void HardDrive::playTone(float freq, uint duration) {
    // TODO: Write own tone function ?
    tone(inputPin, freq, duration);
}

