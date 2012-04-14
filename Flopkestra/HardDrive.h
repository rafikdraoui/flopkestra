#ifndef HARDDRIVE_H
#define HARDDRIVE_H

#include "Instrument.h"

class HardDrive {
public:
    HardDrive(byte inputPin);
    void playTone(float frequency, uint duration);

private:
    byte inputPin;
};

#endif
