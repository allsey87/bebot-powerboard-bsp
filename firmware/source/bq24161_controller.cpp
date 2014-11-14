
#include "bq24161_controller.h"

#include <HardwareSerial.h>

#include "tw_controller.h"

// Datasheet: http://www.ti.com/lit/ds/symlink/bq24161.pdf

#define BQ24161_ADDR 0x6B

#define STAT_MASK 0x70
#define FAULT_MASK 0x07
#define ADAPTER_STAT_MASK 0xC0
#define USB_STAT_MASK 0x30
#define BATT_STAT_MASK 0x06

#define R0_WDT_RST_MASK 0x80

#define R2_RST_MASK 0x80
#define R2_USB_INPUT_LIMIT_MASK 0x70


void CBQ24161Controller::ResetWatchdogTimer() {
   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = CTWController::GetInstance().Read();

unRegVal |= R0_WDT_RST_MASK;

   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().Write(unRegVal);
   CTWController::GetInstance().EndTransmission(true);

}

void CBQ24161Controller::DumpRegister(uint8_t un_addr) {
   HardwareSerial::instance().write("Register(");
   HardwareSerial::instance().write('0' + un_addr);
   HardwareSerial::instance().write(")\r\n");
   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(un_addr);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24161_ADDR, 1, true);
   CTWController::GetInstance().Read();
}

void CBQ24161Controller::SetUSBInputLimit(EUSBInputLimit e_usb_input_limit) {
   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(0x02);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = CTWController::GetInstance().Read();

/* clear the reset bit, always set on read */
unRegVal &= ~R2_RST_MASK;
/* clear the USB input limit bits */
unRegVal &= ~R2_USB_INPUT_LIMIT_MASK;

   switch(e_usb_input_limit) {
 case EUSBInputLimit::L100:
unRegVal |= (0 << 4);
      break;
 case EUSBInputLimit::L150:
unRegVal |= (1 << 4);
      break;
 case EUSBInputLimit::L500:
unRegVal |= (2 << 4);
      break;
 case EUSBInputLimit::L800:
unRegVal |= (3 << 4);
      break;
 case EUSBInputLimit::L900:
unRegVal |= (4 << 4);
      break;
 case EUSBInputLimit::L1500:
unRegVal |= (5 << 4);
      break;
   }

/* write back */
   CTWController::GetInstance().BeginTransmission(BQ24161_ADDR);
   CTWController::GetInstance().Write(0x02);
   CTWController::GetInstance().Write(unRegVal);
   CTWController::GetInstance().EndTransmission(true);

}

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
