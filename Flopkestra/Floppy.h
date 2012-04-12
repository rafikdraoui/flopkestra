#ifndef FLOPPY_H
#define FLOPPY_H

#include "Instrument.h"

class Floppy : public Instrument {
public:
    Floppy(byte driveSelectPin, byte dirPin, byte stepPin);
    virtual void playTone(float frequency, int duration);

private:
    byte driveSelectPin, dirPin, stepPin;
};

#endif
