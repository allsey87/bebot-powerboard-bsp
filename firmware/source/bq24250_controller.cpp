
#include "bq24250_controller.h"

#include <stdint.h>
#include <HardwareSerial.h>

#include "tw_controller.h"

// Datasheet: http://www.ti.com/lit/ds/symlink/bq24250.pdf

#define BQ24250_ADDR 0x6A

#define R0_STAT_MASK 0x30
#define R0_FAULT_MASK 0x0F
#define R0_WDEN_MASK 0x40
#define R0_WDFAULT_MASK 0x80

#define R1_ILIMIT_MASK 0x70
#define R1_HIZ_MASK 0x01
#define R1_RST_MASK 0x80
#define R1_CHGEN_MASK 0x02

void CBQ24250Controller::DumpRegister(uint8_t un_addr) {
   HardwareSerial::instance().write("Register(");
   HardwareSerial::instance().write('0' + un_addr);
   HardwareSerial::instance().write(")\r\n");
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(un_addr);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);
   CTWController::GetInstance().Read();
}

void CBQ24250Controller::SetRegisterValue(uint8_t un_addr, uint8_t un_mask, uint8_t un_value) {
   /* read old value */
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(un_addr);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);
   uint8_t unRegister = CTWController::GetInstance().Read();
   /* clear bits to be updated */
   unRegister &= ~un_mask;
   /* shift the value into the correct position */
   while((un_mask & 0x01) == 0) {
      un_mask >>= 1;
      un_value <<= 1;
   }
   /* set the updated bits */
   unRegister |= un_value;
   /* write back the value */
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(un_addr);
   CTWController::GetInstance().Write(unRegister);
   CTWController::GetInstance().EndTransmission(true);
}

uint8_t CBQ24250Controller::GetRegisterValue(uint8_t un_addr, uint8_t un_mask) {
   /* read old value */
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(un_addr);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);
   uint8_t unRegister = CTWController::GetInstance().Read();
   /* clear unwanted bits */
   unRegister &= un_mask;
   /* shift value down to the correction position  */
   while((un_mask & 0x01) == 0) {
      un_mask >>= 1;
      unRegister >>= 1;
   }
   /* return the result */
   return unRegister;
}

void CBQ24250Controller::SetInputCurrentLimit(EInputCurrentLimit eInputCurrentLimit) {
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x01);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);

   uint8_t unRegister = CTWController::GetInstance().Read();

   /* clear the current value and assure reset is clear */
   unRegister &= ~R1_ILIMIT_MASK;
   unRegister &= ~R1_HIZ_MASK;
   unRegister &= ~R1_RST_MASK;

   switch(eInputCurrentLimit) {
   case EInputCurrentLimit::L100:
      unRegister |= (0x00 << 4);
      break;
   case EInputCurrentLimit::L150:
      unRegister |= (0x01 << 4);
      break;
   case EInputCurrentLimit::L500:
      unRegister |= (0x02 << 4);
      break;
   case EInputCurrentLimit::L900:
      unRegister |= (0x03 << 4);
      break;
   case EInputCurrentLimit::L1500:
      unRegister |= (0x04 << 4);
      break;
   case EInputCurrentLimit::L2000:
      unRegister |= (0x05 << 4);
      break;
   case EInputCurrentLimit::LEXT:
      unRegister |= (0x06 << 4);
      break;
   case EInputCurrentLimit::LPTM:
      unRegister |= (0x07 << 4);
      break;
   case EInputCurrentLimit::LHIZ:
      unRegister |= R1_HIZ_MASK;
      break;
   }

   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x01);
   CTWController::GetInstance().Write(unRegister);
   CTWController::GetInstance().EndTransmission(true);

   //DEBUG

   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x01);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);
   unRegister = CTWController::GetInstance().Read();
}


void CBQ24250Controller::ResetWatchdogTimer() {
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().Write(0x40);
   CTWController::GetInstance().EndTransmission(true);

   // DEBUG
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);
   CTWController::GetInstance().Read();
}


void CBQ24250Controller::EnableCharging() {
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x01);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);

   uint8_t unRegister = CTWController::GetInstance().Read();

   /* assure reset is clear */
   unRegister &= ~R1_RST_MASK;

   /* enable charging */
   unRegister &= ~R1_CHGEN_MASK;

   /* write back */
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x01);
   CTWController::GetInstance().Write(unRegister);
   CTWController::GetInstance().EndTransmission(true);

}

void CBQ24250Controller::Synchronize() {
   CTWController::GetInstance().BeginTransmission(BQ24250_ADDR);
   CTWController::GetInstance().Write(0x00);
   CTWController::GetInstance().EndTransmission(false);
   CTWController::GetInstance().Read(BQ24250_ADDR, 1, true);

   uint8_t unRegister = CTWController::GetInstance().Read();

   /* update the device state variable */
   switch((unRegister & R0_STAT_MASK) >> 4) {
   case 0x00:
      eDeviceState = EDeviceState::READY;
      break;
   case 0x01:
      eDeviceState = EDeviceState::CHARGING;
      break;
   case 0x02:
      eDeviceState = EDeviceState::DONE;
      break;
   case 0x03:
      eDeviceState = EDeviceState::FAULT;
      break;
   }  
   
   /* udpate the fault variable */
   switch(unRegister & R0_FAULT_MASK) {
   case 0x00:
      eFault = EFault::NONE;
      break;
   case 0x01:
      eFault = EFault::INPUT_OVER_VOLTAGE;
      break;
   case 0x02:
      eFault = EFault::INPUT_UNDER_VOLTAGE;
      break;
   case 0x03:
      eFault = EFault::SLEEP;
      break;
   case 0x04:
      eFault = EFault::BATT_THERMAL_SHDN;
      break;
   case 0x05:
      eFault = EFault::BATT_OVER_VOLTAGE;
      break;
   case 0x06:
      eFault = EFault::DEV_THERMAL_SHDN;
      break;
   case 0x07:
      eFault = EFault::DEV_TIMER_FAULT;
      break;
   case 0x08:
      eFault = EFault::BATT_DISCONNECTED;
      break;
   case 0x09:
      eFault = EFault::ISET_SHORTED;
      break;
   case 0x0A:
      eFault = EFault::INPUT_FAULT;
      break;
   default:
      eFault = EFault::UNDEFINED;
      break;
   }

   /* Update the watchdog variables */
   bWatchdogEnabled = ((unRegister & R0_WDEN_MASK) != 0);
   bWatchdogFault = ((unRegister & R0_WDFAULT_MASK) != 0);
}

