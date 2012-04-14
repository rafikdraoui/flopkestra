#ifndef HARDDRIVE_H
#define HARDDRIVE_H

#include "Instrument.h"

class HardDrive : public Instrument {
public:
    HardDrive(byte inputPin);
    virtual void playTone(float frequency, uint duration);

private:
    byte inputPin;
};

#endif
