#ifndef PACKET_CONTROL_INTERFACE_H
#define PACKET_CONTROL_INTERFACE_H

#include <huart_controller.h>

#define RX_COMMAND_BUFFER_LENGTH 16

#define PREAMBLE1  0xF0
#define PREAMBLE2  0xCA
#define POSTAMBLE1 0x53
#define POSTAMBLE2 0x0F

#define PREAMBLE_SIZE 2
#define TYPE_FIELD_SIZE 1
#define DATA_LENGTH_FIELD_SIZE 1
#define CHECKSUM_FIELD_SIZE 1
#define POSTAMBLE_SIZE 2

#define NON_DATA_SIZE (PREAMBLE_SIZE + TYPE_FIELD_SIZE + DATA_LENGTH_FIELD_SIZE + \
                       CHECKSUM_FIELD_SIZE + POSTAMBLE_SIZE)

#define TYPE_OFFSET 2
#define DATA_LENGTH_OFFSET 3
#define DATA_START_OFFSET 4
#define CHECKSUM_OFFSET -3

class CPacketControlInterface {

public:
   enum class EState {
      SRCH_PREAMBLE1,
      SRCH_PREAMBLE2,
      SRCH_POSTAMBLE1,
      SRCH_POSTAMBLE2,
      RECV_COMMAND,
      BUF_OVERFLOW,
   }; 

   class CPacket {
   public:
      
      enum class EType : uint8_t {
         SET_DDS_ENABLE = 0x10,
         SET_DDS_SPEED  = 0x11,
         GET_DDS_SPEED  = 0x12,

         INVALID = 0xFF
      };
      
   public:

      CPacket(uint8_t un_type_id,
              uint8_t un_data_length,
              uint8_t* pun_data) :
         m_unTypeId(un_type_id),
         m_unDataLength(un_data_length),
         m_punData(pun_data) {}
      
      EType GetType() const;
      
      bool HasData() const;
      uint8_t GetDataLength() const;
      const uint8_t* GetDataPointer() const;

   private: 
      uint8_t m_unTypeId;
      uint8_t m_unDataLength;
      uint8_t* m_punData;
   };

public:
   CPacketControlInterface(CHUARTController& c_controller) :
      m_eState(EState::SRCH_PREAMBLE1),
      m_unRxBufferPointer(0),
      m_unUsedBufferLength(0),
      m_unReparseOffset(RX_COMMAND_BUFFER_LENGTH),
      m_cPacket(0xFF, 0, 0),
      m_cController(c_controller) {}

   EState GetState() const;

   const char* StateToString(EState e_state) const;

   const CPacket& GetPacket() const;
  
   void ProcessInput();

   const char* StateToString(EState e_state);

   void Reset();
      
private:
   uint8_t ComputeChecksum();

   void SendAck(const CPacket& c_packet,
                bool b_interpreted = true,
                uint8_t* pun_tx_data = NULL,
                uint8_t un_tx_data_length = 0) const;


   EState m_eState;

   uint8_t m_unRxBufferPointer;
   uint8_t m_unUsedBufferLength;
   uint8_t m_unReparseOffset;
   uint8_t m_punRxBuffer[RX_COMMAND_BUFFER_LENGTH];
   
   CPacket m_cPacket;

   CHUARTController& m_cController;
   

};
   
#endif
   
