
#include "bq24161_controller.h"

#include <stdint.h>
#include <HardwareSerial.h>

#include "tw_controller.h"

// Datasheet: http://www.ti.com/lit/ds/symlink/bq24161.pdf

#define BQ24161_ADDR 0x6B

#define STAT_MASK 0x70
#define FAULT_MASK 0x07
#define ADAPTER_STAT_MASK 0xC0
#define USB_STAT_MASK 0x30
#define BATT_STAT_MASK 0x06


void CBQ24161Controller::Synchronize() {
   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24161_ADDR, 2, true);

   uint8_t punRegisters[2];

   punRegisters[0] = CTWController::GetInstance().Read();
   punRegisters[1] = CTWController::GetInstance().Read();

   /* update the selected source variable */
   switch((punRegisters[0] & STAT_MASK) >> 4) {
   case 0x00:
      eSelectedSource = ESource::NONE;
      eDeviceState = EDeviceState::STANDBY;
      break;
   case 0x01:
      eSelectedSource = ESource::ADAPTER;
      eDeviceState = EDeviceState::READY;
      break;
   case 0x02:
      eSelectedSource = ESource::USB;
      eDeviceState = EDeviceState::READY;
      break;
   case 0x03:
      eSelectedSource = ESource::ADAPTER;
      eDeviceState = EDeviceState::CHARGING;
      break;
   case 0x04:
      eSelectedSource = ESource::USB;
      eDeviceState = EDeviceState::CHARGING;
      break;
   case 0x05:
      eSelectedSource = ESource::NONE;
      eDeviceState = EDeviceState::DONE;
      break;
   case 0x06:
      eSelectedSource = ESource::NONE;
      eDeviceState = EDeviceState::FAULT;
      break;
   case 0x07:
      eSelectedSource = ESource::NONE;
      eDeviceState = EDeviceState::FAULT;
      break;
   }  
   
   /* udpate the fault variable */
   switch(punRegisters[0] & FAULT_MASK) {
   case 0x00:
      eFault = EFault::NONE;
      break;
   case 0x01:
      eFault = EFault::DEV_THERMAL_SHDN;
      break;
   case 0x02:
      eFault = EFault::BATT_THERMAL_SHDN;
      break;
   case 0x03:
      eFault = EFault::WATCHDOG_TMR_EXPR;
      break;
   case 0x04:
      eFault = EFault::SAFETY_TMR_EXPR;
      break;
   case 0x05:
      eFault = EFault::ADAPTER_FAULT;
      break;
   case 0x06:
      eFault = EFault::USB_FAULT;
      break;
   case 0x07:
      eFault = EFault::BATT_FAULT;
      break;
   }

   /* update adapter input status variable */
   switch((punRegisters[1] & ADAPTER_STAT_MASK) >> 6) {
   case 0x00:
      eAdapterInputState = EInputState::NORMAL;
      break;
   case 0x01:
      eAdapterInputState = EInputState::OVER_VOLTAGE;
      break;
   case 0x02:
      eAdapterInputState = EInputState::WEAK_SOURCE;
      break;
   case 0x03:
      eAdapterInputState = EInputState::UNDER_VOLTAGE;
      break;
   }  

   /* update USB input status variable */
   switch((punRegisters[1] & USB_STAT_MASK) >> 4) {
   case 0x00:
      eUSBInputState = EInputState::NORMAL;
      break;
   case 0x01:
      eUSBInputState = EInputState::OVER_VOLTAGE;
      break;
   case 0x02:
      eUSBInputState = EInputState::WEAK_SOURCE;
      break;
   case 0x03:
      eUSBInputState = EInputState::UNDER_VOLTAGE;
      break;
   }

   /* update battery status variable */
   switch((punRegisters[1] & BATT_STAT_MASK) >> 1) {
   case 0x00:
      eBatteryState = EBatteryState::NORMAL;
      break;
   case 0x01:
      eBatteryState = EBatteryState::OVER_VOLTAGE;
      break;
   case 0x02:
      eBatteryState = EBatteryState::DISCONNECTED;
      break;
   case 0x03:
      eBatteryState = EBatteryState::UNDEFINED;
      break;
   }
}
