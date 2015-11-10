
#include "usb2532_module.h"

#include <firmware.h>

#define HUB_CFGMODE_ADDR 0x2D

#define STRLEN_BYTES 3
#define LANGID_BYTES 2

void CUSB2532Module::Init() {
   /* configure the USB hub and attach */
   const char16_t pchManufacturer[] = u"SCT Paderborn";
   const char16_t pchProduct[] = u"BeBot";
   const char16_t pchSerial[] = u"BB129";
   
   /* calculate length of strings minus the null terminating character */
   uint8_t unManufacturerStrLen = sizeof(pchManufacturer) - sizeof(char16_t);
   uint8_t unProductStrLen = sizeof(pchProduct) - sizeof(char16_t);
   uint8_t unSerialStrLen = sizeof(pchSerial) - sizeof(char16_t);
   
   uint8_t punBuffer[64];
   uint8_t unBufferIdx = 0;
   /* write configuration register */
   punBuffer[unBufferIdx++] = 0x00;
   /* number of bytes to be written (strings + 3 bytes giving the length of each string) */
   punBuffer[unBufferIdx++] = LANGID_BYTES + STRLEN_BYTES + unManufacturerStrLen + unProductStrLen + unSerialStrLen;
   /* starting address MSB */
   punBuffer[unBufferIdx++] = 0x30;
   /* starting address LSB */   
   punBuffer[unBufferIdx++] = 0x11;

   /* write language identifier (US English) MSB */
   punBuffer[unBufferIdx++] = 0x04;
   /* write language identifier (US English) LSB */
   punBuffer[unBufferIdx++] = 0x09;

   /* write length of manufacturer string */   
   punBuffer[unBufferIdx++] = unManufacturerStrLen / 2;
   /* write length of product string */   
   punBuffer[unBufferIdx++] = unProductStrLen / 2;
   /* write length of serial string */   
   punBuffer[unBufferIdx++] = unSerialStrLen / 2;

   for(uint8_t unIdx = 0; unIdx < unManufacturerStrLen; unIdx++) {
      punBuffer[unBufferIdx++] = reinterpret_cast<const uint8_t*>(pchManufacturer)[unIdx];
   }
   for(uint8_t unIdx = 0; unIdx < unProductStrLen; unIdx++) {
      punBuffer[unBufferIdx++] = reinterpret_cast<const uint8_t*>(pchProduct)[unIdx];
   }
   for(uint8_t unIdx = 0; unIdx < unSerialStrLen; unIdx++) {
      punBuffer[unBufferIdx++] = reinterpret_cast<const uint8_t*>(pchSerial)[unIdx];
   }

   /* write the configuration to memory */
   Write(0x0000, unBufferIdx, punBuffer);

   /* transfer the configuration from memory to registers */
   Write(static_cast<uint16_t>(ECommand::EXEC_REG_OP));


   /* Enable string support */
   unBufferIdx = 0;
   punBuffer[unBufferIdx++] = 0x00;
   punBuffer[unBufferIdx++] = 0x01;
   /* starting address MSB */
   punBuffer[unBufferIdx++] = 0x30;
   /* starting address LSB */   
   punBuffer[unBufferIdx++] = 0x08;

   punBuffer[unBufferIdx++] = 0x01;

      /* write the configuration to memory */
   Write(0x0000, unBufferIdx, punBuffer);

   /* transfer the configuration from memory to registers */
   Write(static_cast<uint16_t>(ECommand::EXEC_REG_OP));


   /* Read value back */  
   unBufferIdx = 0;  
   punBuffer[unBufferIdx++] = 0x01;
   punBuffer[unBufferIdx++] = 0x30;
   /* starting address MSB */
   punBuffer[unBufferIdx++] = 0x30;
   /* starting address LSB */   
   punBuffer[unBufferIdx++] = 0x11;
   Write(0x0000, unBufferIdx, punBuffer);
   Write(static_cast<uint16_t>(ECommand::EXEC_REG_OP));
   Read(0x0004, 48, punBuffer);
      
   /* exit configuration stage and connect the hub */
   Write(static_cast<uint16_t>(ECommand::HUB_ATTACH));
}

void CUSB2532Module::Write(uint16_t un_offset, uint8_t un_count, uint8_t* pun_buffer) {
   CFirmware::GetInstance().GetTWController().BeginTransmission(HUB_CFGMODE_ADDR);
   CFirmware::GetInstance().GetTWController().Write(un_offset >> 8);
   CFirmware::GetInstance().GetTWController().Write(un_offset & 0xFF);
   CFirmware::GetInstance().GetTWController().Write(un_count);
   for(uint8_t unIdx = 0; unIdx < un_count; unIdx++) {
      CFirmware::GetInstance().GetTWController().Write(pun_buffer[unIdx]);
   }
   CFirmware::GetInstance().GetTWController().EndTransmission(true);
}

void CUSB2532Module::Read(uint16_t un_offset, uint8_t un_count, uint8_t* pun_buffer) {
   CFirmware::GetInstance().GetTWController().BeginTransmission(HUB_CFGMODE_ADDR);
   CFirmware::GetInstance().GetTWController().Write(un_offset >> 8);
   CFirmware::GetInstance().GetTWController().Write(un_offset & 0xFF);
   CFirmware::GetInstance().GetTWController().EndTransmission(false);
   CFirmware::GetInstance().GetTWController().Read(HUB_CFGMODE_ADDR, un_count, true);
   fprintf(CFirmware::GetInstance().m_psHUART, "R:");
   for(uint8_t unIdx = 0; unIdx < un_count; unIdx++) {
      pun_buffer[unIdx] = CFirmware::GetInstance().GetTWController().Read();
      fprintf(CFirmware::GetInstance().m_psHUART, "0x%02x:", pun_buffer[unIdx]);
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
}
