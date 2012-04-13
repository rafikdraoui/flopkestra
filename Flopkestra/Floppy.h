#ifndef FLOPPY_H
#define FLOPPY_H

#include "Instrument.h"

class Floppy : public Instrument {
public:
    Floppy(byte driveSelectPin, byte stepPin, byte dirPin);
    virtual void playTone(float frequency, int duration);

private:
    byte driveSelectPin, stepPin, dirPin;
};

#endif
