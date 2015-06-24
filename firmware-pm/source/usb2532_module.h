#ifndef USB2532_MODULE_H
#define USB2532_MODULE_H

#include <stdint.h>

class CUSB2532Module {
public:
   void Init();
   void Write(uint16_t un_offset, uint8_t un_count = 0, uint8_t* pun_buffer = 0);
   void Read(uint16_t un_offset, uint8_t un_count, uint8_t* pun_buffer);

private:
   enum class ECommand : uint16_t {
      EXEC_REG_OP = 0x9937,
      HUB_ATTACH = 0xAA55,
      OTP_PROG = 0x9933,
      OTP_READ = 0x9934
   };
};

#endif
