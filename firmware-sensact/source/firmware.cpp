#include "firmware.h"
#include "interrupt.h"

/* initialisation of the static singleton */
CFirmware CFirmware::_firmware;

/* main function that runs the firmware */
int main(void)
{
   /* FILE structs for fprintf */
   FILE huart;

   /* Set up FILE structs for fprintf */                           
   fdev_setup_stream(&huart, 
                     [](char c_to_write, FILE* pf_stream) {
                        CFirmware::GetInstance().GetHUARTController().Write(c_to_write);
                        return 1;
                     },
                     [](FILE* pf_stream) {
                        return int(CFirmware::GetInstance().GetHUARTController().Read());
                     },
                     _FDEV_SETUP_RW);

   CFirmware::GetInstance().SetFilePointer(&huart);

   /* Execute the firmware */
   CFirmware::GetInstance().Exec();

   /* Shutdown */
   return 0;
}

/***********************************************************/
/***********************************************************/

void CFirmware::Exec() {
   m_cAccelerometerSystem.Init();

   for(;;) {
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
            if(cPacket.GetDataLength() == 4) {
               const uint8_t* punRxData = cPacket.GetDataPointer();
               int16_t nLeftVelocity = 0, nRightVelocity = 0;
               
               reinterpret_cast<uint16_t&>(nLeftVelocity) = (punRxData[0] << 8) | punRxData[1];
               reinterpret_cast<uint16_t&>(nRightVelocity) = (punRxData[2] << 8) | punRxData[3];
               m_cDifferentialDriveSystem.SetTargetVelocity(nLeftVelocity, nRightVelocity);
            }
            break;
         case CPacketControlInterface::CPacket::EType::GET_DDS_SPEED:
            if(cPacket.GetDataLength() == 0) {
               /* Get the speed of the differential drive system */               
               int16_t nLeftSpeed = m_cDifferentialDriveSystem.GetLeftVelocity();
               int16_t nRightSpeed = m_cDifferentialDriveSystem.GetRightVelocity();
               uint8_t punTxData[] {
                  reinterpret_cast<uint8_t*>(&nLeftSpeed)[1],
                  reinterpret_cast<uint8_t*>(&nLeftSpeed)[0],
                  reinterpret_cast<uint8_t*>(&nRightSpeed)[1],
                  reinterpret_cast<uint8_t*>(&nRightSpeed)[0],
               };
               m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED,
                                                    punTxData,
                                                    sizeof(punTxData));
            }
            break;
         case CPacketControlInterface::CPacket::EType::GET_UPTIME:
            if(cPacket.GetDataLength() == 0) {
               /* timer not implemented to improve interrupt latency for the shaft encoders */
               uint8_t punTxData[] = {0, 0, 0, 0};
               m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_UPTIME,
                                                    punTxData,
                                                    4);
            }
            break;
         case CPacketControlInterface::CPacket::EType::GET_ACCEL_READING:
            if(cPacket.GetDataLength() == 0) {
               CAccelerometerSystem::SReading sReading = m_cAccelerometerSystem.GetReading();
               uint8_t punTxData[] = {
                  uint8_t((sReading.X >> 8) & 0xFF),
                  uint8_t((sReading.X >> 0) & 0xFF),
                  uint8_t((sReading.Y >> 8) & 0xFF),
                  uint8_t((sReading.Y >> 0) & 0xFF),
                  uint8_t((sReading.Z >> 8) & 0xFF),
                  uint8_t((sReading.Z >> 0) & 0xFF),
                  uint8_t((sReading.Temp >> 8) & 0xFF),
                  uint8_t((sReading.Temp >> 0) & 0xFF),                  
               };
               m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::GET_ACCEL_READING,
                                                    punTxData,
                                                    sizeof(punTxData));
            }
            break;
         default:
            /* unknown command */
            break;
         }
      }
      /* Upload data */
      if(m_bNewData) {
         /* block interrupts */
         uint8_t unSREG = SREG;
         cli();
         /* copy data */
         int16_t m_nLeftTargetCpy = m_nLeftTarget;
         int16_t m_nLeftErrorCpy = m_nLeftError;
         int16_t nLeftErrorDerivativeCpy = nLeftErrorDerivative;
         float m_fLeftErrorIntegralCpy = m_fLeftErrorIntegral;
         float fLeftOutputCpy = fLeftOutput;
         uint8_t unLeftDutyCycleCpy = unLeftDutyCycle;
         int16_t m_nRightTargetCpy = m_nRightTarget;
         int16_t m_nRightErrorCpy = m_nRightError;
         int16_t nRightErrorDerivativeCpy = nRightErrorDerivative;
         float m_fRightErrorIntegralCpy = m_fRightErrorIntegral;
         float fRightOutputCpy = fRightOutput;
         uint8_t unRightDutyCycleCpy = unRightDutyCycle;
         m_bNewData = false;
         /* restore interrupts */
         SREG = unSREG;
         /* tx the data */
         uint32_t Marker = 0xABCDEFFF;
         uint8_t punTxData[] = {
            reinterpret_cast<uint8_t*>(&Marker)[0],
            reinterpret_cast<uint8_t*>(&Marker)[1],
            reinterpret_cast<uint8_t*>(&Marker)[2],
            reinterpret_cast<uint8_t*>(&Marker)[3],
            reinterpret_cast<uint8_t*>(&m_nLeftTargetCpy)[0],
            reinterpret_cast<uint8_t*>(&m_nLeftTargetCpy)[1],
            reinterpret_cast<uint8_t*>(&m_nLeftErrorCpy)[0],
            reinterpret_cast<uint8_t*>(&m_nLeftErrorCpy)[1],
            reinterpret_cast<uint8_t*>(&nLeftErrorDerivativeCpy)[0],
            reinterpret_cast<uint8_t*>(&nLeftErrorDerivativeCpy)[1],
            reinterpret_cast<uint8_t*>(&m_fLeftErrorIntegralCpy)[0],
            reinterpret_cast<uint8_t*>(&m_fLeftErrorIntegralCpy)[1],
            reinterpret_cast<uint8_t*>(&m_fLeftErrorIntegralCpy)[2],
            reinterpret_cast<uint8_t*>(&m_fLeftErrorIntegralCpy)[3],
            reinterpret_cast<uint8_t*>(&fLeftOutputCpy)[0],
            reinterpret_cast<uint8_t*>(&fLeftOutputCpy)[1],
            reinterpret_cast<uint8_t*>(&fLeftOutputCpy)[2],
            reinterpret_cast<uint8_t*>(&fLeftOutputCpy)[3],
            unLeftDutyCycleCpy,
            reinterpret_cast<uint8_t*>(&m_nRightTargetCpy)[0],
            reinterpret_cast<uint8_t*>(&m_nRightTargetCpy)[1],
            reinterpret_cast<uint8_t*>(&m_nRightErrorCpy)[0],
            reinterpret_cast<uint8_t*>(&m_nRightErrorCpy)[1],
            reinterpret_cast<uint8_t*>(&nRightErrorDerivativeCpy)[0],
            reinterpret_cast<uint8_t*>(&nRightErrorDerivativeCpy)[1],
            reinterpret_cast<uint8_t*>(&m_fRightErrorIntegralCpy)[0],
            reinterpret_cast<uint8_t*>(&m_fRightErrorIntegralCpy)[1],
            reinterpret_cast<uint8_t*>(&m_fRightErrorIntegralCpy)[2],
            reinterpret_cast<uint8_t*>(&m_fRightErrorIntegralCpy)[3],
            reinterpret_cast<uint8_t*>(&fRightOutputCpy)[0],
            reinterpret_cast<uint8_t*>(&fRightOutputCpy)[1],
            reinterpret_cast<uint8_t*>(&fRightOutputCpy)[2],
            reinterpret_cast<uint8_t*>(&fRightOutputCpy)[3],
            unRightDutyCycleCpy
         };
         m_cPacketControlInterface.SendPacket(CPacketControlInterface::CPacket::EType::INVALID,
                                              punTxData,
                                              sizeof(punTxData));
      }
   }
}

/***********************************************************/
/***********************************************************/
