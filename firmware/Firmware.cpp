#include "Firmware.h"

/* Instantiate the firmware */
Firmware firmware;

/* Static initialization of the timer */
Timer Firmware::timer;

/* Static initialization of the serial port */
HardwareSerial Firmware::serial(&Firmware::timer);

/* Run the firmware */
int main(void)
{
   return firmware.exec();
}

