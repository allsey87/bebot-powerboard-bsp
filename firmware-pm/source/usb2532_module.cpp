
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
   /* number of bytes to be written (strings + 3 bytes giving the length of each strings) */
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
   //Write(0x0000, unBufferIdx, punBuffer);

   /* transfer the configuration from memory to registers */
   //Write(static_cast<uint16_t>(ECommand::EXEC_REG_OP));

   /* exit configuration stage and connect the hub */
   Write(static_cast<uint16_t>(ECommand::HUB_ATTACH));
}

void CUSB2532Module::Write(uint16_t un_offset, uint8_t un_count, uint8_t* pun_buffer) {
   Firmware::GetInstance().GetTWController().BeginTransmission(HUB_CFGMODE_ADDR);
   Firmware::GetInstance().GetTWController().Write(un_offset >> 8);
   Firmware::GetInstance().GetTWController().Write(un_offset & 0xFF);
   Firmware::GetInstance().GetTWController().Write(un_count);
   for(uint8_t unIdx = 0; unIdx < un_count; unIdx++) {
      Firmware::GetInstance().GetTWController().Write(pun_buffer[unIdx]);
   }
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}

void CUSB2532Module::Read(uint16_t un_offset, uint8_t un_count, uint8_t* pun_buffer) {
   Firmware::GetInstance().GetTWController().BeginTransmission(HUB_CFGMODE_ADDR);
   Firmware::GetInstance().GetTWController().Write(un_offset >> 8);
   Firmware::GetInstance().GetTWController().Write(un_offset & 0xFF);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(HUB_CFGMODE_ADDR, un_count, true);
   for(uint8_t unIdx = 0; unIdx < un_count; unIdx++) {
      pun_buffer[unIdx] = Firmware::GetInstance().GetTWController().Read();
   }
}
