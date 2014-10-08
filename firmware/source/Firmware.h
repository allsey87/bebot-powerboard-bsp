#ifndef FIRMWARE_H
#define FIRMWARE_H

/* AVR Headers */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Firmware Headers */
//#include <Timer.h>
#include <HardwareSerial.h>
#include <tw_controller.h>
#include <bq24161_controller.h>

/* I2C Address Space */
#define MPU6050_ADDR               0x68

enum class EMPU6050Register : uint8_t {
   /* MPU6050 Registers */
   PWR_MGMT_1     = 0x6B, // R/W
   PWR_MGMT_2     = 0x6C, // R/W
   ACCEL_XOUT_H   = 0x3B, // R  
   ACCEL_XOUT_L   = 0x3C, // R  
   ACCEL_YOUT_H   = 0x3D, // R  
   ACCEL_YOUT_L   = 0x3E, // R  
   ACCEL_ZOUT_H   = 0x3F, // R  
   ACCEL_ZOUT_L   = 0x40, // R  
   TEMP_OUT_H     = 0x41, // R  
   TEMP_OUT_L     = 0x42, // R  
   WHOAMI         = 0x75  // R
};

class Firmware {
public:
      
   static Firmware& instance() {
      return _firmware;
   }

   int exec() {
      HardwareSerial::instance().write("Booted\r\n");
      Timer::instance().delay(500);
      DDRJ = 0x80;
      PORTJ = 0x80;      
      HardwareSerial::instance().write("PWRON\r\n");
      Timer::instance().delay(500);

      for(;;) {
         if(HardwareSerial::instance().available()) {
            TestBQ24161();
            HardwareSerial::instance().write("\r\n");

            while(HardwareSerial::instance().available()) {
               HardwareSerial::instance().read();
            }
         }
      }
      return 0;         
   }
      
private:

   Firmware() {
      // Enable interrupts
      sei();      
   }
 
   void TestBQ24161();  

CBQ24161Controller cBQ24161Controller;

   static Firmware _firmware;
};

#endif
