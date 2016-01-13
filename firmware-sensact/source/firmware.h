#ifndef FIRMWARE_H
#define FIRMWARE_H

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
#include <packet_control_interface.h>
#include <timer.h>

#include <differential_drive_system.h>
#include <accelerometer_system.h>

class Firmware {
public:
   static Firmware& GetInstance() {
      return _firmware;
   }

   void SetFilePointer(FILE* ps_huart) {
      m_psHUART = ps_huart;
   }

   CHUARTController& GetHUARTController() {
      return m_cHUARTController;
   }

   CTWController& GetTWController() {
      return m_cTWController;
   }

   CTimer& GetTimer() {
      return m_cTimer;
   }

   /* TESTING */
   int8_t nRightTarget = 0;
   int16_t nRightError = 0;
   int16_t nRightErrorIntegral = 0;
   int16_t nRightErrorDerivative = 0;
   int16_t nRightOutput = 0;
   
   int8_t nLeftTarget = 0;
   int16_t nLeftError = 0;
   int16_t nLeftErrorIntegral = 0;
   int16_t nLeftErrorDerivative = 0;
   int16_t nLeftOutput = 0;
   /* TESTING */

   void Exec() {
      m_cAccelerometerSystem.Init();

      for(;;) {
           
         if(TIMSK0 & (1 << TOIE0)) {
            uint8_t unSREG = SREG;
            cli();
            uint32_t unTime = m_cTimer.GetMilliseconds();
            uint8_t punSomeData[] = {
               uint8_t((unTime >> 24) & 0xFF),
               uint8_t((unTime >> 16) & 0xFF),
               uint8_t((unTime >> 8 ) & 0xFF),
               uint8_t((unTime >> 0 ) & 0xFF),
               reinterpret_cast<const uint8_t&>(nRightTarget),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightError) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightError) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightErrorIntegral) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightErrorIntegral) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightErrorDerivative) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightErrorDerivative) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightOutput) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nRightOutput) & 0xFF),
               reinterpret_cast<const uint8_t&>(nLeftTarget),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftError) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftError) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftErrorIntegral) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftErrorIntegral) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftErrorDerivative) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftErrorDerivative) & 0xFF),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftOutput) >> 8),
               uint8_t(reinterpret_cast<const uint16_t&>(nLeftOutput) & 0xFF),
            };
            SREG = unSREG;
            /*
            m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_PARAMS,
                                                 punSomeData,
                                               22);
                                               */
         }

         /* TESTING */
      
         m_cPacketControlInterface.ProcessInput();

         if(m_cPacketControlInterface.GetState() == CPacketControlInterface::EState::RECV_COMMAND) {
            CPacketControlInterface::CPacket cPacket = m_cPacketControlInterface.GetPacket();
            switch(cPacket.GetType()) {
            case CPacketControlInterface::CPacket::EType::SET_DDS_ENABLE:
               /* Set the enable signal for the differential drive system */
               if(cPacket.GetDataLength() == 1) {
                  const uint8_t* punRxData = cPacket.GetDataPointer();
                  if(punRxData[0] == 0) {
                     m_cDifferentialDriveSystem.Disable();
                  }
                  else {
                     m_cDifferentialDriveSystem.Enable();
                  }
               }
               break;
            case CPacketControlInterface::CPacket::EType::SET_DDS_SPEED:
               /* Set the speed of the differential drive system */
               if(cPacket.HasData() && cPacket.GetDataLength() == 2) {
                  const uint8_t* punRxData = cPacket.GetDataPointer();
                  int8_t nLeftVelocity = reinterpret_cast<const int8_t&>(punRxData[0]);
                  int8_t nRightVelocity = reinterpret_cast<const int8_t&>(punRxData[1]);
                  m_cDifferentialDriveSystem.SetTargetVelocity(nLeftVelocity, nRightVelocity);
               }
               break;
            case CPacketControlInterface::CPacket::EType::GET_DDS_SPEED:
               if(cPacket.GetDataLength() == 0) {
                  uint8_t punTxData[2];
                  /* Get the speed of the differential drive system */
                  reinterpret_cast<int8_t&>(punTxData[0]) = m_cDifferentialDriveSystem.GetLeftVelocity();
                  reinterpret_cast<int8_t&>(punTxData[1]) = m_cDifferentialDriveSystem.GetRightVelocity();
                  m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED,
                                                       punTxData,
                                                       2);
               }
               break;
            case CPacketControlInterface::CPacket::EType::SET_DDS_PARAMS:
               /* Set the parameters of the differential drive system controller */
               if(cPacket.GetDataLength() == 4) {
                  const uint8_t* punRxData = cPacket.GetDataPointer();
                  uint8_t unSREG = SREG;
                  cli();
                  m_cDifferentialDriveSystem.m_cPIDControlStepInterrupt.m_unKp = punRxData[0];
                  m_cDifferentialDriveSystem.m_cPIDControlStepInterrupt.m_unKi = punRxData[1];
                  m_cDifferentialDriveSystem.m_cPIDControlStepInterrupt.m_unKd = punRxData[2];
                  m_cDifferentialDriveSystem.m_cPIDControlStepInterrupt.m_unScale = punRxData[3];
                  SREG = unSREG;
               }
               break;
            case CPacketControlInterface::CPacket::EType::GET_UPTIME:
               if(cPacket.GetDataLength() == 0) {
                  uint32_t unUptime = m_cTimer.GetMilliseconds();
                  uint8_t punTxData[] = {
                     uint8_t((unUptime >> 24) & 0xFF),
                     uint8_t((unUptime >> 16) & 0xFF),
                     uint8_t((unUptime >> 8 ) & 0xFF),
                     uint8_t((unUptime >> 0 ) & 0xFF)
                  };
                  m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_UPTIME,
                                                       punTxData,
                                                       4);
               }
               break;
               /*
            case CPacketControlInterface::CPacket::EType::GET_ACCEL_READING:
               {
                  CAccelerometerSystem::SReading sReading = m_cAccelerometerSystem.GetReading();
                  reinterpret_cast<void&>(punTxData[0]) = sReading.X;
                  reinterpret_cast<int8_t&>(punTxData[2]) = sReading.Y;
                  reinterpret_cast<int16_t&>(punTxData[4]) = sReading.Z;
                  reinterpret_cast<int16_t&>(punTxData[6]) = sReading.Temp;
                  m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_ACCEL_READING,
                                                       punTxData,
                                                       8);
               }
               break;
               */
            default:
               /* unknown command */
               break;
            }
         }
      }
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
      m_cHUARTController(CHUARTController::instance()),
      m_cTWController(CTWController::GetInstance()),
      m_cPacketControlInterface(m_cHUARTController) {     

      /* Enable interrupts */
      sei();
   }
   
   CTimer m_cTimer;

   /* ATMega328P Controllers */
   CHUARTController& m_cHUARTController;
 
   CTWController& m_cTWController;
public:
   CPacketControlInterface m_cPacketControlInterface;

   /* Core system functional units */
   CDifferentialDriveSystem m_cDifferentialDriveSystem;

   CAccelerometerSystem m_cAccelerometerSystem;

   static Firmware _firmware;

public: // TODO, don't make these public
    /* File structs for fprintf */
   FILE* m_psHUART;

};

#endif
