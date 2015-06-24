#ifndef FIRMWARE_H
#define FIRMWARE_H

//#define DEBUG

/* AVR Headers */
#include <avr/io.h>
#include <avr/interrupt.h>

/* debug */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Firmware Headers */
#include <huart_controller.h>
#include <tw_controller.h>
#include <timer.h>

#include <differential_drive_system.h>

class Firmware {
public:
      
   static Firmware& GetInstance() {
      return _firmware;
   }

   void SetFilePointer(FILE* ps_huart) {
      m_psHUART = ps_huart;
   }

   /*
   CHUARTController& GetHUARTController() {
      return m_cHUARTController;
   }
   */
   HardwareSerial& GetHUARTController() {
      return m_cHUARTController;
   }

   CTimer& GetTimer() {
      return m_cTimer;
   }

   int Exec() {
      enum class EMotor { LEFT, RIGHT } eSelectedMotor = EMotor::RIGHT;
      CDifferentialDriveSystem::SVelocity sVelocity;
      int16_t nLeftSpeed = 0, nRightSpeed = 0;
      uint8_t unInput = 0;
      for(;;) {
         
         if(Firmware::GetInstance().GetHUARTController().Available()) {
            unInput = Firmware::GetInstance().GetHUARTController().Read();
            /* flush */
            while(Firmware::GetInstance().GetHUARTController().Available()) {
               Firmware::GetInstance().GetHUARTController().Read();
            }
         }
         else {
            unInput = 0;
         }
         switch(unInput) {
         case 'u':
            fprintf(m_psHUART, "Uptime: %lums\r\n", m_cTimer.GetMilliseconds());
            break;
         case 'l':
            fprintf(m_psHUART, "Selected Left\r\n");
            eSelectedMotor = EMotor::LEFT;
            break;
         case 'r':
            fprintf(m_psHUART, "Selected Right\r\n");
            eSelectedMotor = EMotor::RIGHT;
            break;          
         case 'e':
            fprintf(m_psHUART, "DDS Enabled\r\n");
            m_cDifferentialDriveSystem.Enable();
            break;
         case '0':
            fprintf(m_psHUART, "DDS Disabled\r\n");
            m_cDifferentialDriveSystem.Disable();
            break;
         case '1' ... '9':
            if(eSelectedMotor == EMotor::RIGHT) {
               nRightSpeed = (unInput - '5') * 25;
            }
            else {
               nLeftSpeed = (unInput - '5') * 25;
            }
            fprintf(m_psHUART, "Target: L:%d\tR:%d\r\n", nLeftSpeed, nRightSpeed);
            m_cDifferentialDriveSystem.SetTargetVelocity(nLeftSpeed, nRightSpeed);
            break;
         case 's':
            sVelocity = m_cDifferentialDriveSystem.GetVelocity();
            fprintf(m_psHUART, 
                    "L:%d\tR:%d\r\n", 
                    sVelocity.Left, 
                    sVelocity.Right);
            break;
         case 't':
            fprintf(m_psHUART, "-- DDS Test Start --\r\n");
            TestDriveSystem();
            fprintf(m_psHUART, "-- DDS Test End --\r\n");
            break;

         default:
            break;
         }
      }
      

      return 0;
   }

   void TestDriveSystem() {
      int16_t pnTestVelocities[] = {0, 40, 120, -80, 0};
      m_cDifferentialDriveSystem.Enable();
      for(int16_t nTargetVelocity : pnTestVelocities) {
         m_cDifferentialDriveSystem.SetTargetVelocity(nTargetVelocity, nTargetVelocity);
         for(uint16_t cnt = 0; cnt < 400; cnt++) {
            CDifferentialDriveSystem::SVelocity sVelocity = 
               m_cDifferentialDriveSystem.GetVelocity();
            fprintf(m_psHUART, 
                    "%4d\t%4d\t%4d\r\n", 
                    nTargetVelocity, 
                    sVelocity.Left, 
                    sVelocity.Right);
         }
      }
      m_cDifferentialDriveSystem.Disable();
   }
      
private:

   /* private constructor */
   Firmware() :
      m_cTimer(TCCR2A,
               TCCR2A | (1 << WGM21) | (1 << WGM20),
               TCCR2B,
               TCCR2B | (1 << CS22),
               TIMSK2,
               TIMSK2 | (1 << TOIE2),
               TIFR2,
               TCNT2,
               TIMER2_OVF_vect_num),
      m_cHUARTController(HardwareSerial::instance()),
      m_cTWController(CTWController::GetInstance()) {     

      /* Enable interrupts */
      sei();
   }
   
   CTimer m_cTimer;

   /* ATMega328P Controllers */
   /* TODO remove singleton and reference from HUART */
   //CHUARTController& m_cHUARTController;
   HardwareSerial& m_cHUARTController;
 
   CTWController& m_cTWController;

   CDifferentialDriveSystem m_cDifferentialDriveSystem;

   static Firmware _firmware;

public: // TODO, don't make these public
    /* File structs for fprintf */
   FILE* m_psHUART;

};

#endif
