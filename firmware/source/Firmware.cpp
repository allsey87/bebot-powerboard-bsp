#include "Firmware.h"
#include <bq24161_controller.h>
/* initialisation of the static singleton */
Firmware Firmware::_firmware;

const char INVALID_PARAM[] = "Invalid Parameter\r\n";

/* main function that runs the firmware */
int main(void)
{
   /* FILE structs for fprintf */
   FILE huart;

   /* Set up FILE struct for fprintf */                           
   fdev_setup_stream(&huart, 
                     [](char c_to_write, FILE* pf_stream) {
                        HardwareSerial::instance().write(c_to_write);
                        return 1;
                     },
                     [](FILE* pf_stream) {
                        return int(HardwareSerial::instance().read());
                     },
                     _FDEV_SETUP_RW);

   Firmware::instance().SetFilePointer(&huart);

   return Firmware::instance().exec();
}

void Firmware::SetWatchdogPeriod(char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "%u", &unVal) == 1) {
      unWatchdogPeriod = unVal;
      fprintf(m_psIOFile, "Watchdog period set to %u\r\n", unWatchdogPeriod);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::ReadRegister(char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0x07) {
      cBQ24250Controller.DumpRegister(unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetVDPMTo4V2(char* pun_args) {
   cBQ24250Controller.SetRegisterValue(0x04, R4_VDPM_MASK, 0);
   fprintf(m_psIOFile, "BQ24250 VDPM set to 4.2V\r\n");
}

void Firmware::CheckFaults(char* pun_args) {
   fprintf(m_psIOFile, "LED Fault: %c\r\n", (PINJ & 0x04)==0?'T':'F');
   fprintf(m_psIOFile, "MTR Fault: %c\r\n", (PINJ & 0x10)==0?'T':'F');
   fprintf(m_psIOFile, "USB Fault: %c\r\n", (PINJ & 0x01)==0?'T':'F');
}

void Firmware::SetLEDsMaxCurrent(char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0xFF) {
      cMAX5419Controller.SetActualValue(unVal);
      fprintf(m_psIOFile, "Maximum LED current set to %u\r\n", unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}


void Firmware::SetSystemEnable(char* pun_args) {
   if(pun_args != NULL && strstr(pun_args, "on") != NULL) {
      DDRJ |= 0x80;
      PORTJ |= 0x80;
      fprintf(m_psIOFile, "System Enable ON\r\n");
   }
   else if(pun_args != NULL && strstr(pun_args, "off") != NULL) {
      DDRJ |= 0x80;
      PORTJ &= ~0x80;      
      fprintf(m_psIOFile, "System Enable OFF\r\n");
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetLEDsEnable(char* pun_args) {
   if(pun_args != NULL && strstr(pun_args, "on") != NULL) {
      DDRJ |= 0x08;
      PORTJ |= 0x08;      
      fprintf(m_psIOFile, "LEDs Enable ON\r\n");
   }
   else if(pun_args != NULL && strstr(pun_args, "off") != NULL) {
      DDRJ |= 0x08;
      PORTJ &= ~0x08;
      fprintf(m_psIOFile, "LEDs Enable OFF\r\n");
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetBQ24250InputEnable(char* pun_args) {
   if(pun_args != NULL && strstr(pun_args, "on") != NULL) {
      DDRE |= 0x80;
      PORTE |= 0x80;
      fprintf(m_psIOFile, "BQ24250 Input ON\r\n");
   }
   else if(pun_args != NULL && strstr(pun_args, "off") != NULL) {
      DDRE |= 0x80;
      PORTE &= ~0x80;
      fprintf(m_psIOFile, "BQ24250 Input OFF\r\n");
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetBQ24250InputCurrent(char* pun_args) {
   if(pun_args != NULL && strstr(pun_args, "100") != NULL) {
      cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L100);
      fprintf(m_psIOFile, "BQ24250 Input Current %s\r\n", pun_args);
   }
   else if(pun_args != NULL && strstr(pun_args, "HIZ") != NULL) {
      cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::LHIZ);
      fprintf(m_psIOFile, "BQ24250 Input Current %s\r\n", pun_args);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetUSBCurrent(char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "%u", &unVal) == 1) {
      if(unVal == 100 || unVal == 500) {
         DDRJ |= 0x02;
         if(unVal == 100) {
            PORTJ &= ~0x02; 
         }
         else {
            PORTJ |= 0x02;      
         }
         fprintf(m_psIOFile, "Setting USB current to %d\r\n", unVal);
         return;
      }
   }
   fprintf(m_psIOFile, INVALID_PARAM);
}

void Firmware::TestPMIC(char* pun_args) {
   uint16_t unPartNum = 0;
   if(pun_args != NULL && sscanf(pun_args, "BQ%u", &unPartNum) == 1) {
      if(unPartNum == 24250) {
         cBQ24250Controller.Synchronize();
         HardwareSerial::instance().write("BQ24250:");
         HardwareSerial::instance().write("\r\nstate: ");
         switch(cBQ24250Controller.GetDeviceState()) {
         case CBQ24250Controller::EDeviceState::READY:
            HardwareSerial::instance().write("ready");
            break;
         case CBQ24250Controller::EDeviceState::CHARGING:
            HardwareSerial::instance().write("charging");
            break;
         case CBQ24250Controller::EDeviceState::DONE:
            HardwareSerial::instance().write("done");
            break;
         case CBQ24250Controller::EDeviceState::FAULT:
            HardwareSerial::instance().write("fault");
            break;
         }
         HardwareSerial::instance().write("\r\nfault: ");
         switch(cBQ24250Controller.GetFault()) {
         case CBQ24250Controller::EFault::NONE:
            HardwareSerial::instance().write("none");
            break;
         case CBQ24250Controller::EFault::INPUT_OVER_VOLTAGE:
            HardwareSerial::instance().write("input_over_voltage");
            break;
         case CBQ24250Controller::EFault::INPUT_UNDER_VOLTAGE:
            HardwareSerial::instance().write("input_under_voltage");
            break;
         case CBQ24250Controller::EFault::SLEEP:
            HardwareSerial::instance().write("sleep");
            break;
         case CBQ24250Controller::EFault::BATT_THERMAL_SHDN:
            HardwareSerial::instance().write("batt_thermal_shdn");
            break;
         case CBQ24250Controller::EFault::BATT_OVER_VOLTAGE:
            HardwareSerial::instance().write("batt_over_voltage");
            break;
         case CBQ24250Controller::EFault::DEV_THERMAL_SHDN:
            HardwareSerial::instance().write("dev_thermal_shdn");
            break;
         case CBQ24250Controller::EFault::DEV_TIMER_FAULT:
            HardwareSerial::instance().write("dev_timer_fault");
            break;
         case CBQ24250Controller::EFault::BATT_DISCONNECTED:
            HardwareSerial::instance().write("batt_disconnected");
            break;
         case CBQ24250Controller::EFault::ISET_SHORTED:
            HardwareSerial::instance().write("iset_shorted");
            break;
         case CBQ24250Controller::EFault::INPUT_FAULT:
            HardwareSerial::instance().write("input_fault");
            break;
         case CBQ24250Controller::EFault::UNDEFINED:
            HardwareSerial::instance().write("undefined");
            break;
         }
         HardwareSerial::instance().write("\r\nwatchdog enabled = ");
         HardwareSerial::instance().write(cBQ24250Controller.GetWatchdogEnabled()?'t':'f');
         HardwareSerial::instance().write("\r\nwatchdog fault = ");
         HardwareSerial::instance().write(cBQ24250Controller.GetWatchdogFault()?'t':'f');
         HardwareSerial::instance().write("\r\n");
         return;
      }
      else if(unPartNum == 24161) {
         cBQ24161Controller.Synchronize();
         HardwareSerial::instance().write("BQ24161:");
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
         HardwareSerial::instance().write("\r\n");
         return;
      }
   }
   fprintf(m_psIOFile, INVALID_PARAM);
}
