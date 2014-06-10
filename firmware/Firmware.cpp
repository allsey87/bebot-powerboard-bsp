#include "Firmware.h"

/* Instantiate the firmware */
Firmware firmware;

/* Static initialization of the timer */
Timer Firmware::timer;

/* Static initialization of the serial port */
HardwareSerial Firmware::serial(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0, UDRIE0, U2X0);

/* Run the firmware */
int main(void)
{
   return firmware.exec();
}

