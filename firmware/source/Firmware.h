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
#include <bq24250_controller.h>

/* I2C Address Space */
#define MPU6050_ADDR               0x68


// TEMP
#define R4_VDPM_MASK 0x07
#define R5_SYSOFF_MASK 0x10
#define R5_TSEN_MASK 0x08
#define R6_FORCEPTM_MASK 0x04
#define R1_RST_MASK 0x80


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
      for(;;) {
         if(HardwareSerial::instance().available()) {
            switch(uint8_t unInput = HardwareSerial::instance().read()) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
               cBQ24250Controller.DumpRegister(unInput - '0');
               break;
            case 'a':
               TestBQ24161();
               break;
            case 'b':
               TestBQ24250();
               break;
            case 'E':
               HardwareSerial::instance().write("SYS_EN ON\r\n");
               DDRJ |= 0x80;
               PORTJ |= 0x80;      
               break;
            case 'e':
               HardwareSerial::instance().write("SYS_EN OFF\r\n");
               DDRJ |= 0x80;
               PORTJ &= ~0x80;      
               break;
            case 'F':
               HardwareSerial::instance().write("BQ24250_IN ON\r\n");
               DDRE |= 0x80;
               PORTE |= 0x80;
               break;
            case 'f':
               HardwareSerial::instance().write("BQ24250_IN OFF\r\n");
               DDRE |= 0x80;
               PORTE &= ~0x80;
               break;
            case 'P':
               HardwareSerial::instance().write("SET_IN_L500\r\n");
               cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L500);
               break;
            case 'p':
               HardwareSerial::instance().write("SET_IN_HIZ\r\n");
               cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::LHIZ);
               break;
            case 'r':
               HardwareSerial::instance().write("RST_BQ24250_WDT\r\n");
               cBQ24250Controller.ResetWatchdogTimer();
               break;
            case 'R':
               HardwareSerial::instance().write("RST_BQ24250_DEVICE\r\n");
               cBQ24250Controller.SetRegisterValue(0x01, R1_RST_MASK, 1);
               break;
            case 'W':
               HardwareSerial::instance().write("VDPM 4200mV\r\n");
               cBQ24250Controller.SetRegisterValue(0x04, R4_VDPM_MASK, 0);
               break;
            case 'X':
               HardwareSerial::instance().write("SYSOFF ON\r\n");
               cBQ24250Controller.SetRegisterValue(0x05, R5_SYSOFF_MASK, 1);
               break;
            case 'x':
               HardwareSerial::instance().write("SYSOFF OFF\r\n");
               cBQ24250Controller.SetRegisterValue(0x05, R5_SYSOFF_MASK, 0);
               break;
            case 'Y':
               HardwareSerial::instance().write("TS ON\r\n");
               cBQ24250Controller.SetRegisterValue(0x05, R5_TSEN_MASK, 1);
               break;
            case 'y':
               HardwareSerial::instance().write("TS OFF\r\n");
               cBQ24250Controller.SetRegisterValue(0x05, R5_TSEN_MASK, 0);
               break;
            case 'Z':
               HardwareSerial::instance().write("PTM ON\r\n");
               cBQ24250Controller.SetRegisterValue(0x06, R6_FORCEPTM_MASK, 1);
               break;
            case 'z':
               HardwareSerial::instance().write("PTM OFF\r\n");
               cBQ24250Controller.SetRegisterValue(0x06, R6_FORCEPTM_MASK, 0);
               break;
            }
            while(HardwareSerial::instance().available()) {
               HardwareSerial::instance().read();
            }
            HardwareSerial::instance().write("\r\n");
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
   void TestBQ24250();

   CBQ24161Controller cBQ24161Controller;
   CBQ24250Controller cBQ24250Controller;

   static Firmware _firmware;
};

#endif
