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

   void Exec() {
      uint8_t punTxData[8];

      m_cAccelerometerSystem.Init();

      for(;;) {
         m_cPacketControlInterface.ProcessInput();

         if(m_cPacketControlInterface.GetState() == CPacketControlInterface::EState::RECV_COMMAND) {
            CPacketControlInterface::CPacket cPacket = m_cPacketControlInterface.GetPacket();
            switch(cPacket.GetType()) {
            case CPacketControlInterface::CPacket::EType::SET_DDS_ENABLE: 
               fprintf(Firmware::GetInstance().m_psHUART, "SET_DDS_ENABLE: ");
               /* Set the enable signal for the differential drive system */
               if(cPacket.HasData() && cPacket.GetDataLength() == 1) {
                  const uint8_t* punRxData = cPacket.GetDataPointer();
                  fprintf(Firmware::GetInstance().m_psHUART, "%c\r\n", (punRxData[0] == 0)?'0':'1');
                  if(punRxData[0] == 0) {
                     m_cDifferentialDriveSystem.Disable();
                  }
                  else {
                     m_cDifferentialDriveSystem.Enable();
                  }
               }
               break;
            case CPacketControlInterface::CPacket::EType::SET_DDS_SPEED:
               fprintf(Firmware::GetInstance().m_psHUART, "SET_DDS_SPEED: ");
               /* Set the speed of the differential drive system */
               if(cPacket.HasData() && cPacket.GetDataLength() == 2) {
                  const uint8_t* punRxData = cPacket.GetDataPointer();
                  int8_t nLeftVelocity = reinterpret_cast<const int8_t&>(punRxData[0]);
                  int8_t nRightVelocity = reinterpret_cast<const int8_t&>(punRxData[1]);
                  fprintf(Firmware::GetInstance().m_psHUART, "%d %d\r\n", nLeftVelocity, nRightVelocity);
                  m_cDifferentialDriveSystem.SetTargetVelocity(nLeftVelocity, nRightVelocity);
                  //m_cPacketCommandInterface.SendAck(cRxCommand, true);
               }
               break;
            case CPacketControlInterface::CPacket::EType::GET_DDS_SPEED:
               /* Get the speed of the differential drive system */
               reinterpret_cast<int8_t&>(punTxData[0]) = m_cDifferentialDriveSystem.GetLeftVelocity();
               reinterpret_cast<int8_t&>(punTxData[1]) = m_cDifferentialDriveSystem.GetRightVelocity();
               m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED,
                                                    punTxData,
                                                    2);
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
            //m_cPacketControlInterface.Reset();
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
