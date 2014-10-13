
#include "max5419_controller.h"

#include <tw_controller.h>

#define VREG_COMMAND   0x11
#define NVREG_COMMAND  0x21
#define RECALL_COMMAND 0x61

/****************************************/
/****************************************/
 
void CMAX5419Controller::SetActualValue(uint8_t un_val) {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   CTWController::GetInstance().Write(VREG_COMMAND);
   CTWController::GetInstance().Write(un_val);
   CTWController::GetInstance().EndTransmission(true);
}

/****************************************/
/****************************************/

void CMAX5419Controller::SetStoredValue(uint8_t un_val) {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   CTWController::GetInstance().Write(NVREG_COMMAND);
   CTWController::GetInstance().Write(un_val);
   CTWController::GetInstance().EndTransmission(true);
}


/****************************************/
/****************************************/

void CMAX5419Controller::Reset() {
   CTWController::GetInstance().BeginTransmission(m_unAddr);
   CTWController::GetInstance().Write(RECALL_COMMAND);
   CTWController::GetInstance().EndTransmission(true);
}
