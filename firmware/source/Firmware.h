#ifndef FIRMWARE_H
#define FIRMWARE_H

/* AVR Headers */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Firmware Headers */
//#include <Timer.h>
#include <HardwareSerial.h>
#include <tw_controller.h>

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

   bool InitMPU6050();

   int exec() {
      HardwareSerial::instance().write("Booted\r\n");
      Timer::instance().delay(2000);
      DDRJ = 0x80;
      PORTJ = 0x80;      
      HardwareSerial::instance().write("PWRON\r\n");
      Timer::instance().delay(2000);
      CTWController::GetInstance().BeginTransmission(MPU6050_ADDR);
      CTWController::GetInstance().Write(static_cast<uint8_t>(EMPU6050Register::WHOAMI));
      CTWController::GetInstance().EndTransmission(false);
      CTWController::GetInstance().Read(MPU6050_ADDR, 1, true);

      char mpu6050r = CTWController::GetInstance().Read();

      if(mpu6050r == MPU6050_ADDR) {
         HardwareSerial::instance().write("MPU6050_ADDR detected\r\n");
      }
      else {
         HardwareSerial::instance().write("Fail ");
         HardwareSerial::instance().write(mpu6050r);
      }

      for(;;) {
         if(HardwareSerial::instance().available()) {
            HardwareSerial::instance().write("hello world\r\n");
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

   static Firmware _firmware;
};

#endif
