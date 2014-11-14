
#include "mcp23008_module.h"

#include <Firmware.h>

CMCP23008Module::CMCP23008Module(uint8_t un_addr) {
   m_unAddr = un_addr;
}

uint8_t CMCP23008Module::GetRegister(ERegister e_register) {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   CTWController::GetInstance().Write(static_cast<uint8_t>(e_register));
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(m_unAddr, 1, true);
   return CTWController::GetInstance().Read();
}

void CMCP23008Module::SetRegister(ERegister e_register, uint8_t un_val) {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   CTWController::GetInstance().Write(static_cast<uint8_t>(e_register));
   CTWController::GetInstance().Write(un_val);
   CTWController::GetInstance().EndTransmission(true);
}
