#ifndef FIRMWARE_H
#define FIRMWARE_H

/* AVR Headers */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Firmware Headers */
#include <Timer.h>
#include <HardwareSerial.h>

class Firmware {
   public:
      Firmware() {
         // Enable interrupts
         sei();
      
         // Disconnect UART (reconnected by serial.begin())
         UCSR0B = 0;
         
         // Start-up Serial
         serial.begin(115200);
      }
      
      int exec() {         
         for(;;) {
            if(serial.available()) {
               serial.write("hello world\n");
               while(serial.available()) serial.read();
            }
         }
         return 0;         
      }
      
   private:
      static Timer timer;
      static HardwareSerial serial;
};

#endif
