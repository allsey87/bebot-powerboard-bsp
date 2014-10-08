#include "Firmware.h"
#include <bq24161_controller.h>

/* initialisation of the static singleton */
Firmware Firmware::_firmware;

/* main function that runs the firmware */
int main(void)
{
   return Firmware::instance().exec();
}

void Firmware::TestBQ24161() {
   cBQ24161Controller.Synchronize();

   HardwareSerial::instance().write("\r\nstate: ");
   switch(cBQ24161Controller.GetDeviceState()) {
   case CBQ24161Controller::EDeviceState::STANDBY:
      HardwareSerial::instance().write("standby");
      break;
   case CBQ24161Controller::EDeviceState::READY:
      HardwareSerial::instance().write("ready");
      break;
   case CBQ24161Controller::EDeviceState::CHARGING:
      HardwareSerial::instance().write("charging");
      break;
   case CBQ24161Controller::EDeviceState::DONE:
      HardwareSerial::instance().write("done");
      break;
   case CBQ24161Controller::EDeviceState::FAULT:
      HardwareSerial::instance().write("fault");
      break;
   }

   HardwareSerial::instance().write("\r\nfault: ");
   switch(cBQ24161Controller.GetFault()) {
   case CBQ24161Controller::EFault::NONE:
      HardwareSerial::instance().write("none");
      break;
   case CBQ24161Controller::EFault::DEV_THERMAL_SHDN:
      HardwareSerial::instance().write("dev_thermal_shdn");
      break;
   case CBQ24161Controller::EFault::BATT_THERMAL_SHDN:
      HardwareSerial::instance().write("batt_thermal_shdn");
      break;
   case CBQ24161Controller::EFault::WATCHDOG_TMR_EXPR:
      HardwareSerial::instance().write("watchdog_tmr_expr");
      break;
   case CBQ24161Controller::EFault::SAFETY_TMR_EXPR:
      HardwareSerial::instance().write("safety_tmr_expr");
      break;
   case CBQ24161Controller::EFault::ADAPTER_FAULT:
      HardwareSerial::instance().write("adapter_fault");
      break;
   case CBQ24161Controller::EFault::USB_FAULT:
      HardwareSerial::instance().write("usb_fault");
      break;
   case CBQ24161Controller::EFault::BATT_FAULT:
      HardwareSerial::instance().write("batt_fault");
      break;
   }
            
   HardwareSerial::instance().write("\r\nselected source: ");
   switch(cBQ24161Controller.GetSelectedSource()) {
   case CBQ24161Controller::ESource::NONE:
      HardwareSerial::instance().write("none");
      break;
   case CBQ24161Controller::ESource::ADAPTER:
      HardwareSerial::instance().write("adapter");
      break;
   case CBQ24161Controller::ESource::USB:
      HardwareSerial::instance().write("usb");
      break;
   }

   HardwareSerial::instance().write("\r\nadapter input: ");
   switch(cBQ24161Controller.GetAdapterInputState()) {
   case CBQ24161Controller::EInputState::NORMAL:
      HardwareSerial::instance().write("normal");
      break;
   case CBQ24161Controller::EInputState::OVER_VOLTAGE:
      HardwareSerial::instance().write("over_voltage");
      break;
   case CBQ24161Controller::EInputState::WEAK_SOURCE:
      HardwareSerial::instance().write("weak_source");
      break;
   case CBQ24161Controller::EInputState::UNDER_VOLTAGE:
      HardwareSerial::instance().write("under_voltage");
      break;
   }

   HardwareSerial::instance().write("\r\nusb input: ");
   switch(cBQ24161Controller.GetUSBInputState()) {
   case CBQ24161Controller::EInputState::NORMAL:
      HardwareSerial::instance().write("normal");
      break;
   case CBQ24161Controller::EInputState::OVER_VOLTAGE:
      HardwareSerial::instance().write("over_voltage");
      break;
   case CBQ24161Controller::EInputState::WEAK_SOURCE:
      HardwareSerial::instance().write("weak_source");
      break;
   case CBQ24161Controller::EInputState::UNDER_VOLTAGE:
      HardwareSerial::instance().write("under_voltage");
      break;
   }

   HardwareSerial::instance().write("\r\nbattery_state: ");
   switch(cBQ24161Controller.GetBatteryState()) {
   case CBQ24161Controller::EBatteryState::NORMAL:
      HardwareSerial::instance().write("normal");
      break;
   case CBQ24161Controller::EBatteryState::OVER_VOLTAGE:
      HardwareSerial::instance().write("over_voltage");
      break;
   case CBQ24161Controller::EBatteryState::DISCONNECTED:
      HardwareSerial::instance().write("disconnected");
      break;
   case CBQ24161Controller::EBatteryState::UNDEFINED:
      HardwareSerial::instance().write("undefined");
      break;
   }
}
