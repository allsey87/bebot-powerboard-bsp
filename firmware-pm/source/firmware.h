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
#include <bq24161_controller.h>
#include <bq24250_controller.h>
#include <huart_controller.h>
#include <tw_controller.h>
#include <timer.h>

#define PIN_ACTUATORS_EN 0x80
#define PIN_SYSTEM_EN 0x10
#define PIN_BQ24250_INPUT_EN 0x40
#define PIN_VUSB50_L500_EN 0x20

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

   CTWController& GetTWController() {
      return m_cTWController;
   }

   CTimer& GetTimer() {
      return m_cTimer;
   }

   int Exec();

   void TestPMICs();
      
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

   CBQ24161Controller cBQ24161Controller;
   CBQ24250Controller cBQ24250Controller;

   /* ATMega328P Controllers */
   /* TODO remove singleton and reference from HUART */
   //CHUARTController& m_cHUARTController;
   HardwareSerial& m_cHUARTController;
 
   CTWController& m_cTWController;

   static Firmware _firmware;

public: // TODO, don't make these public
    /* File structs for fprintf */
   FILE* m_psHUART;

};

#endif
