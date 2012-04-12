#include "HardDrive.h"

/*
 * The hard drive acts the same way as a regular speaker. The input pin is
 * the signal and the other pin is connected to ground.
 */

HardDrive::HardDrive(byte inputpin) {
    inputPin = inputpin;
}

void HardDrive::playTone(float freq, int duration) {
    // Note: using tone means that only one pin can be used at a time
    // TODO: Write own tone function
    tone(inputPin, freq, duration);
}
