#ifndef FIRMWARE_H
#define FIRMWARE_H

/* AVR Headers */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Firmware Headers */
#include <Timer.h>
#include <HardwareSerial.h>
#include <tw_controller.h>
#include <bq24161_controller.h>
#include <bq24250_controller.h>
#include <max5419_controller.h>
#include <differential_drive_controller.h>

/* I2C Address Space */
#define MPU6050_ADDR           0x68
#define MAX5419_LEDS_ADDR      0x29
#define MAX5419_MTRS_ADDR      0x28

// TEMP
#define R4_VDPM_MASK 0x07
#define R5_SYSOFF_MASK 0x10
#define R5_TSEN_MASK 0x08
#define R6_FORCEPTM_MASK 0x04
#define R1_RST_MASK 0x80

#define KEY_BACKSPACE 0x7F
#define KEY_TAB 0x09

//enum class EMPU6050Register : uint8_t {
enum {
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

#define INPUT_BUFFER_LENGTH 40

class Firmware {
public:

   void ReadEncoders(const char* pun_args);
   void SetMotors(const char* pun_args); 

   void SetLEDsMaxCurrent(const char* pun_args);
   void SetMotorsMaxCurrent(const char* pun_args);
   void SetUSBMaxCurrent(const char* pun_args);
   void SetLEDsEnable(const char* pun_args);
   void SetMotorsEnable(const char* pun_args);
   void SetSystemEnable(const char* pun_args);
   void SetBQ24250VDPMTo4V2(const char* pun_args);
   void SetBQ24250InputCurrent(const char* pun_args);
   void SetBQ24250InputEnable(const char* pun_args);
   void GetBQ24250Register(const char* pun_args);
   void TestPMIC(const char* pun_args);  
   void CheckFaults(const char* pun_args);
   void SetWatchdogPeriod(const char* pun_args);

   struct SCommand {
      char Label[INPUT_BUFFER_LENGTH];
      void (Firmware::*Method)(const char* pun_args);
   } psCommands[16] {
      {"ReadEncoders", &Firmware::ReadEncoders},
      {"SetMotors", &Firmware::SetMotors},
      {"SetLEDsMaxCurrent", &Firmware::SetLEDsMaxCurrent},
      {"SetMotorsMaxCurrent", &Firmware::SetMotorsMaxCurrent},
      {"SetUSBMaxCurrent", &Firmware::SetUSBMaxCurrent},
      {"SetLEDsEnable", &Firmware::SetLEDsEnable},
      {"SetMotorsEnable", &Firmware::SetMotorsEnable},
      {"SetSystemEnable", &Firmware::SetSystemEnable},
      {"SetBQ24250VDPMTo4V2", &Firmware::SetBQ24250VDPMTo4V2},
      {"SetBQ24250InputCurrent", &Firmware::SetBQ24250InputCurrent},
      {"SetBQ24250InputEnable", &Firmware::SetBQ24250InputEnable},
      {"GetBQ24250Register", &Firmware::GetBQ24250Register},
      {"TestPMIC", &Firmware::TestPMIC},
      {"CheckFaults", &Firmware::CheckFaults},
      {"SetWatchdogPeriod", &Firmware::SetWatchdogPeriod},
      {"EndOfArray", NULL}};

   int exec() {
      uint32_t unLastReset = 0;
      char punInputBuffer[INPUT_BUFFER_LENGTH] = {};
      uint8_t unInputBufferIdx = 0;
      char punLastInputBuffer[INPUT_BUFFER_LENGTH] = {};
      uint8_t unLastInputBufferIdx = 0;

      //uint8_t unBackspaceCount = 0;
      uint8_t unInputChar = '\0';
      bool bParseCommand = false;
      unWatchdogPeriod = 0;
      
      fprintf(m_psIOFile, "Booted\r\n");
      for(;;) {
         if(HardwareSerial::instance().available()) {
            while(HardwareSerial::instance().available()) {
               unInputChar = HardwareSerial::instance().read();
               if(unInputChar == KEY_BACKSPACE) {
                  if(unInputBufferIdx > 0) {
                     unInputBufferIdx--;
                  }
               }
               else if(unInputChar == KEY_TAB) {
                  unInputBufferIdx = unLastInputBufferIdx;
                  strcpy(punInputBuffer, punLastInputBuffer);
                  fprintf(m_psIOFile, "\r\n%s", punInputBuffer);
               }
               else {
                  /* put the input character into the buffer */
                  punInputBuffer[unInputBufferIdx] = unInputChar;
                  /* if it was a character return, we are done, add the null terminating character and break */
                  if(punInputBuffer[unInputBufferIdx] == '\n' || punInputBuffer[unInputBufferIdx] == '\r') {
                     punInputBuffer[unInputBufferIdx] = '\0';
                     unLastInputBufferIdx = unInputBufferIdx;
                     strcpy(punLastInputBuffer, punInputBuffer);
                     bParseCommand = true;
                     /* consume the rest of the characters in the buffer */
                     while(HardwareSerial::instance().available())
                        HardwareSerial::instance().read();
                     break;
                  }
                  else {
                     /* move forwards to the next position in the buffer */
                     unInputBufferIdx++;
                     /* if we are at the end of the buffer, add the terminating character and break */
                     if(unInputBufferIdx >= (INPUT_BUFFER_LENGTH - 1)) {
                        fprintf(m_psIOFile, "\r\nError: Input Buffer Overflow\r\n");
                        unInputBufferIdx = 0;
                        /* consume the rest of the characters in the buffer */
                        while(HardwareSerial::instance().available())
                           HardwareSerial::instance().read();
                        break;
                     }
                  }
               }
            }
            if(bParseCommand == true) {
               unInputBufferIdx = 0;
               bParseCommand = false;
               fprintf(m_psIOFile, "\r\nInput: %s\r\n", punInputBuffer);
               /* check if the command has arguments */
               char* punArguments = strstr(punInputBuffer, " ");
               /* if the space was found, replace it with a terminating character */
               if(punArguments != NULL)
                  *punArguments = '\0';

               /* try find a matching function */
               for(uint8_t unIdx = 0; psCommands[unIdx].Method != NULL; unIdx++) {
                  if(strcmp(punInputBuffer, psCommands[unIdx].Label) == 0) {
                     if(punArguments != NULL)
                        punArguments += 1;
                     (this->*psCommands[unIdx].Method)(punArguments);
                  }
               }
            }
         }
         if(unWatchdogPeriod != 0 && (Timer::instance().millis() - unLastReset) > unWatchdogPeriod) {
            //HardwareSerial::instance().write("\rResetting Watchdog\r\n");
            //cBQ24250Controller.ResetWatchdogTimer();
            HardwareSerial::instance().write('\r'); ReadEncoders("");
            unLastReset = Timer::instance().millis();
            HardwareSerial::instance().write("\r\n");
            for(uint8_t unIdx = 0; unIdx < unInputBufferIdx; unIdx++) {
               HardwareSerial::instance().write(punInputBuffer[unIdx]);
            }
         }
      }
      return 0;         
   }


   static Firmware& instance() {
      return _firmware;
   }

   void SetFilePointer(FILE* ps_io_file) {
      m_psIOFile = ps_io_file;
   }

   void SetMotor(void (CDifferentialDriveController::*pf_set_motor)
                 (CDifferentialDriveController::EMode, uint8_t),
                 const char* pun_args);
      

      
private:

   Firmware() : 
      cLEDsCurrentController(MAX5419_LEDS_ADDR),
      cMotorsCurrentController(MAX5419_MTRS_ADDR) {
      /* Enable interrupts */
      sei();      
   }

   FILE* m_psIOFile;
   uint16_t unWatchdogPeriod;

   CBQ24161Controller cBQ24161Controller;
   CBQ24250Controller cBQ24250Controller;

   CMAX5419Controller cLEDsCurrentController;
   CMAX5419Controller cMotorsCurrentController;

   CDifferentialDriveController cDifferentialDriveController;

   static Firmware _firmware;
};

#endif
