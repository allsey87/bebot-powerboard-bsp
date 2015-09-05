
#include <firmware.h>

#include "packet_control_interface.h"

CPacketControlInterface::CPacket::EType CPacketControlInterface::CPacket::GetType() const {
   switch(m_unTypeId) {
   case 0x10:
      return EType::SET_DDS_ENABLE;
      break;
   case 0x11:
      return EType::SET_DDS_SPEED;
      break;
   case 0x12:
      return EType::GET_DDS_SPEED;
      break;
   default:
      return EType::INVALID;
      break;
   }
}
      
bool CPacketControlInterface::CPacket::HasData() const {
   return (m_unDataLength != 0);
}

uint8_t CPacketControlInterface::CPacket::GetDataLength() const {
   return m_unDataLength;
}

const uint8_t* CPacketControlInterface::CPacket::GetDataPointer() const {
   return m_punData;
}

CPacketControlInterface::EState CPacketControlInterface::GetState() const {
   return m_eState;
}


void CPacketControlInterface::SendAck(const CPacket& c_packet,
                                      bool b_interpreted,
                                      uint8_t* pun_tx_data,
                                      uint8_t un_tx_data_length) const {

   
}

void CPacketControlInterface::Reset() {
   m_unRxBufferPointer = 0;
   m_unUsedBufferLength = 0;
   m_eState = EState::SRCH_PREAMBLE1;
}
   
void CPacketControlInterface::ProcessInput() {
   bool bBufOverflow = false;
   uint8_t unRxByte = 0;
   uint8_t unReparseOffset = RX_COMMAND_BUFFER_LENGTH;
  
   while(m_eState != EState::RECV_COMMAND) {
      /* Check if the buffer has overflown */
      if(bBufOverflow) {
         //////////////
         fprintf(Firmware::GetInstance().m_psHUART, "Old Buf = [ ");
         for(uint8_t unIdx = 0; unIdx < RX_COMMAND_BUFFER_LENGTH; unIdx++) {
            fprintf(Firmware::GetInstance().m_psHUART, "0x%02x ", m_punRxBuffer[unIdx]);        
         }
         fprintf(Firmware::GetInstance().m_psHUART, "]\r\n");
         //////////////
         /* Search for the beginning of a preamble in buffer */
         // m_unUsedBufferLength => max index of valid data
         //m_unRxBufferPointer 0> current position
         for(unReparseOffset = 1; unReparseOffset < m_unUsedBufferLength; unReparseOffset++) {
            if(m_punRxBuffer[unReparseOffset] == PREAMBLE1)
               break;
         }

         fprintf(Firmware::GetInstance().m_psHUART, "m_unUsedBufferLength = %u, unReparseOffset = %u\r\n", m_unUsedBufferLength, unReparseOffset);


         /* Shift data down in buffer */
         for(uint8_t unBufferIdx = unReparseOffset;
             unBufferIdx < m_unUsedBufferLength;
             unBufferIdx++) {
            fprintf(Firmware::GetInstance().m_psHUART, "*\r\n", RX_COMMAND_BUFFER_LENGTH, unReparseOffset);
            m_punRxBuffer[unBufferIdx - unReparseOffset] = m_punRxBuffer[unBufferIdx];
         }

         m_unUsedBufferLength -= unReparseOffset;

         /* The overflow has been handled */
         bBufOverflow = false;
         /* Reparse the buffer */
         m_unRxBufferPointer = 0;
         m_eState = EState::SRCH_PREAMBLE1;

         //////////////
         fprintf(Firmware::GetInstance().m_psHUART, "New Buf = [ ");
         for(uint8_t unIdx = 0; unIdx < RX_COMMAND_BUFFER_LENGTH; unIdx++) {
            fprintf(Firmware::GetInstance().m_psHUART, "0x%02x ", m_punRxBuffer[unIdx]);        
         }
         fprintf(Firmware::GetInstance().m_psHUART, "]\r\n");
         /////////////////
      }
    
      if(m_unRxBufferPointer < m_unUsedBufferLength) {
         unRxByte = m_punRxBuffer[m_unRxBufferPointer];
         m_punRxBuffer[m_unRxBufferPointer++] = unRxByte;
         fprintf(Firmware::GetInstance().m_psHUART,"unRxByte(R) = 0x%02x\r\n", unRxByte);
      }     
      else if(m_cController.Available()) {
         unRxByte = m_cController.Read();
         m_punRxBuffer[m_unRxBufferPointer++] = unRxByte;
         m_unUsedBufferLength++;
         fprintf(Firmware::GetInstance().m_psHUART,"unRxByte(I) = 0x%02x\r\n", unRxByte);
      }
      else {
         break;
      }

      fprintf(Firmware::GetInstance().m_psHUART, "[State: %s]\r\n", StateToString(m_eState));
      
      fprintf(Firmware::GetInstance().m_psHUART, 
              "m_unUsedBufferLength = %u\r\n"
              "unReparseOffset = %u\r\n"
              "m_unRxBufferPointer = %u\r\n",
              m_unUsedBufferLength, unReparseOffset, m_unRxBufferPointer);


      //////////////
      fprintf(Firmware::GetInstance().m_psHUART, "Buf = [ ");
      for(uint8_t unIdx = 0; unIdx < RX_COMMAND_BUFFER_LENGTH; unIdx++) {
         fprintf(Firmware::GetInstance().m_psHUART, "0x%02x ", m_punRxBuffer[unIdx]);        
      }
      fprintf(Firmware::GetInstance().m_psHUART, "]\r\n");
      /////////////////
   
      switch(m_eState) {
      case EState::SRCH_PREAMBLE1:
         if(unRxByte != PREAMBLE1) {
            bBufOverflow = true;
         }
         else {
            m_eState = EState::SRCH_PREAMBLE2;
         }
         break;
      case EState::SRCH_PREAMBLE2:
         if(unRxByte != PREAMBLE2) {
            bBufOverflow = true;
            /*
            m_unRxBufferPointer = 0;
            m_eState = EState::SRCH_PREAMBLE1;
            */
         }
         else {
            m_eState = EState::SRCH_POSTAMBLE1;
         }
         break;
      case EState::SRCH_POSTAMBLE1:
         if(unRxByte != POSTAMBLE1) {
            m_eState = EState::SRCH_POSTAMBLE1;
         }
         else {
            /* unRxByte == POSTAMBLE1 */
            m_eState = EState::SRCH_POSTAMBLE2;
         }
         break;
      case EState::SRCH_POSTAMBLE2:
         if(unRxByte != POSTAMBLE2) {
            /* previous byte wasn't actually part of postamble */
            if(unRxByte != POSTAMBLE1) {
               m_eState = EState::SRCH_POSTAMBLE1;
            }
            else {
               /* unRxByte == POSTAMBLE1 */
               m_eState = EState::SRCH_POSTAMBLE2;
            }
         }
         else {
            /* unRxByte == POSTAMBLE2 */
            fprintf(Firmware::GetInstance().m_psHUART, 
                    "CS: 0x%02x =? 0x%02x, DL: %u =? %u\r\n",
                    ComputeChecksum(),
                    m_punRxBuffer[m_unRxBufferPointer + CHECKSUM_OFFSET],
                    m_unRxBufferPointer >= NON_DATA_SIZE ? 
                    m_unRxBufferPointer - NON_DATA_SIZE : 0xFF,
                    m_punRxBuffer[DATA_LENGTH_OFFSET]);

            /* check if the length field and checksum are valid */
            if(m_unRxBufferPointer >= NON_DATA_SIZE &&
               m_punRxBuffer[DATA_LENGTH_OFFSET] == (m_unRxBufferPointer - NON_DATA_SIZE) &&
               ComputeChecksum() == m_punRxBuffer[m_unRxBufferPointer + CHECKSUM_OFFSET]) {
               /* At this point we assume we have a valid command */
               m_eState = EState::RECV_COMMAND;
               /* Populate the packet fields */
               m_cPacket = CPacket(m_punRxBuffer[TYPE_OFFSET],
                                   m_punRxBuffer[DATA_LENGTH_OFFSET],
                                   &m_punRxBuffer[DATA_START_OFFSET]);
            }
            else {
               m_eState = EState::SRCH_POSTAMBLE1;
            }  
         }
         break;
      default:
         break;
      }
      bBufOverflow = bBufOverflow || (m_unRxBufferPointer == RX_COMMAND_BUFFER_LENGTH);
   }
}

const char* CPacketControlInterface::StateToString(CPacketControlInterface::EState e_state) {
   switch(e_state) {
   case EState::SRCH_PREAMBLE1:
      return "SRCH_PREAMBLE1";
      break;
   case EState::SRCH_PREAMBLE2:
      return "SRCH_PREAMBLE2";
      break;
   case EState::SRCH_POSTAMBLE1:
      return "SRCH_POSTAMBLE1";
      break;
   case EState::SRCH_POSTAMBLE2:
      return "SRCH_POSTAMBLE2";
      break;
   case EState::RECV_COMMAND:
      return "RECV_COMMAND";
      break;
   default:
      return "UNKNOWN STATE";
      break;
   }
}

const CPacketControlInterface::CPacket& CPacketControlInterface::GetPacket() const {
   return m_cPacket;
}


uint8_t CPacketControlInterface::ComputeChecksum() {
   uint8_t unChecksum = 0;
   for(uint8_t unIdx = TYPE_OFFSET;
       unIdx < DATA_START_OFFSET + m_punRxBuffer[DATA_LENGTH_OFFSET];
       unIdx++) {
      unChecksum += m_punRxBuffer[unIdx];
   }
   return unChecksum;
}
