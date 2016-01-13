#include "firmware.h"

/* initialisation of the static singleton */
Firmware Firmware::_firmware;

/* main function that runs the firmware */
int main(void)
{
   /* FILE structs for fprintf */
   FILE huart;

   /* Set up FILE structs for fprintf */                           
   fdev_setup_stream(&huart, 
                     [](char c_to_write, FILE* pf_stream) {
                        Firmware::GetInstance().GetHUARTController().Write(c_to_write);
                        return 1;
                     },
                     [](FILE* pf_stream) {
                        return int(Firmware::GetInstance().GetHUARTController().Read());
                     },
                     _FDEV_SETUP_RW);

   Firmware::GetInstance().SetFilePointer(&huart);

   /* Execute the firmware */
   Firmware::GetInstance().Exec();

   /* Shutdown */
   return 0;
}

/***********************************************************/
/***********************************************************/

void Firmware::Exec() {
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
            if(cPacket.GetDataLength() == 2) {
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
         case CPacketControlInterface::CPacket::EType::GET_UPTIME:
            if(cPacket.GetDataLength() == 0) {
               /* timer not implemented to improve interrupt latency for PID controller */
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
   }
}

/***********************************************************/
/***********************************************************/
