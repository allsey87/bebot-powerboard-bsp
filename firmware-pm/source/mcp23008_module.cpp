
#include "mcp23008_module.h"

#include <firmware.h>

CMCP23008Module::CMCP23008Module(uint8_t un_addr) {
   m_unAddr = un_addr;
}

uint8_t CMCP23008Module::GetRegister(ERegister e_register) {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   Firmware::GetInstance().GetTWController().Write(static_cast<uint8_t>(e_register));
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(m_unAddr, 1, true);
   return Firmware::GetInstance().GetTWController().Read();
}

void CMCP23008Module::SetRegister(ERegister e_register, uint8_t un_val) {
   Firmware::GetInstance().GetTWController().BeginTransmission(m_unAddr);
   Firmware::GetInstance().GetTWController().Write(static_cast<uint8_t>(e_register));
   Firmware::GetInstance().GetTWController().Write(un_val);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}
