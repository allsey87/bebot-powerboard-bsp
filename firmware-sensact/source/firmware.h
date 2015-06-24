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
      //uint8_t unInput = 0;

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

      
      // for(;;) {
         
      //    if(Firmware::GetInstance().GetHUARTController().Available()) {
      //       unInput = Firmware::GetInstance().GetHUARTController().Read();
      //       /* flush */
      //       while(Firmware::GetInstance().GetHUARTController().Available()) {
      //          Firmware::GetInstance().GetHUARTController().Read();
      //       }
      //    }
      //    else {
      //       unInput = 's';
      //    }

      //    switch(unInput) {
      //    case 'u':
      //       fprintf(m_psHUART, "Uptime = %lums\r\n", m_cTimer.GetMilliseconds());
      //       break;
      //    case 'E':
      //       m_cDifferentialDriveSystem.Enable();
      //       break;
      //    case 'e':
      //       m_cDifferentialDriveSystem.Disable();
      //       break;
      //    case 'l':
      //       eSelectedMotor = EMotor::LEFT;
      //       break;
      //    case 'r':
      //       eSelectedMotor = EMotor::RIGHT;
      //       break;          
      //    case 's':
      //       m_cDifferentialDriveSystem.GetVelocity();
      //       break;          
      //    case '0' ... '9':
      //       m_cDifferentialDriveSystem.SetTargetVelocity((unInput - '5') * 40, (unInput - '5') * 40);
      //       //m_cDifferentialDriveSystem.ConfigureRightMotor(eRightDriveMode, 80 + (unInput - '0') * 5);
      //       break;
      //    default:
      //       break;
      //    }
      // }
      

      return 0;
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
