
#include "bq24161_controller.h"

#include "firmware.h"

// Datasheet: http://www.ti.com/lit/ds/symlink/bq24161.pdf

#define BQ24161_ADDR 0x6B

// TODO move these defines to the base tw class
#define R0_ADDR 0x00
#define R1_ADDR 0x01
#define R2_ADDR 0x02
#define R3_ADDR 0x03
#define R4_ADDR 0x04
#define R5_ADDR 0x05

#define STAT_MASK 0x70
#define FAULT_MASK 0x07
#define ADAPTER_STAT_MASK 0xC0
#define USB_STAT_MASK 0x30
#define BATT_STAT_MASK 0x06

#define R0_WDT_RST_MASK 0x80

#define R1_NOBATT_OP_MASK 0x01
#define R1_OTG_LOCKOUT_MASK 0x08

#define R2_RST_MASK 0x80
#define R2_USB_INPUT_LIMIT_MASK 0x70
#define R2_CHG_EN_MASK 0x02
#define R2_TERM_EN_MASK 0x04

void CBQ24161Controller::ResetWatchdogTimer() {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R0_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

   unRegVal |= R0_WDT_RST_MASK;

   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R0_ADDR);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);

}

void CBQ24161Controller::DumpRegister(uint8_t un_addr) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(un_addr);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);
   fprintf(Firmware::GetInstance().m_psHUART,
           "Register 0x%02x : 0x%02x\r\n",
           un_addr,
           Firmware::GetInstance().GetTWController().Read());
}

void CBQ24161Controller::SetUSBInputLimit(EUSBInputLimit e_usb_input_limit) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R2_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

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
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(0x02);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}


void CBQ24161Controller::SetChargeTerminationEnable(bool b_enable) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R2_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

   /* clear the reset bit, always set on read */
   unRegVal &= ~R2_RST_MASK;
   /* set the charge termination flag with respect to b_enable */
   if(b_enable == true) {
      unRegVal |= R2_TERM_EN_MASK; 
   }
   else {
      unRegVal &= ~R2_TERM_EN_MASK;
   }
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R2_ADDR);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}

void CBQ24161Controller::SetChargingEnable(bool b_enable) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R2_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

   /* clear the reset bit, always set on read */
   unRegVal &= ~R2_RST_MASK;
   /* set the charge enable flag with respect to b_enable */
   if(b_enable == true) {
      unRegVal &= ~R2_CHG_EN_MASK;
   }
   else {
      unRegVal |= R2_CHG_EN_MASK;   
   }
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R2_ADDR);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}

void CBQ24161Controller::SetNoBattOperationEnable(bool b_enable) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R1_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

   /* set the no battery operation flag with respect to b_enable */
   if(b_enable == true) {
      unRegVal |= R1_NOBATT_OP_MASK;
   }
   else {
      unRegVal &= ~R1_NOBATT_OP_MASK;
   }
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R1_ADDR);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}

void CBQ24161Controller::SetUSBLockoutEnable(bool b_enable) {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R1_ADDR);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 1, true);

   uint8_t unRegVal = Firmware::GetInstance().GetTWController().Read();

   /* set the USB OTG lockout flag with respect to b_enable */
   if(b_enable == true) {
      unRegVal |= R1_OTG_LOCKOUT_MASK;
   }
   else {
      unRegVal &= ~R1_OTG_LOCKOUT_MASK;
   }
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(R1_ADDR);
   Firmware::GetInstance().GetTWController().Write(unRegVal);
   Firmware::GetInstance().GetTWController().EndTransmission(true);
}


void CBQ24161Controller::Synchronize() {
   Firmware::GetInstance().GetTWController().BeginTransmission(BQ24161_ADDR);
   Firmware::GetInstance().GetTWController().Write(0x00);
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(BQ24161_ADDR, 2, true);

   uint8_t punRegisters[2];

   punRegisters[0] = Firmware::GetInstance().GetTWController().Read();
   punRegisters[1] = Firmware::GetInstance().GetTWController().Read();

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
