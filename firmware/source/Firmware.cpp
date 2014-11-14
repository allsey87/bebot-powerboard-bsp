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

void Firmware::ReadEncoders(const char* pun_args) {
   fprintf(m_psIOFile,
           "Left RPM = %d, Right RPM = %d\r\n", 
           cDifferentialDriveController.GetLeftRPM(),
           cDifferentialDriveController.GetRightRPM());
}


void Firmware::SetMotors(const char* pun_args) {
   if(pun_args != NULL) {
      const char* punLeftMotorArgs = strstr(pun_args, "l:");
      const char* punRightMotorArgs = strstr(pun_args, "r:");
      if(punLeftMotorArgs != NULL) {
         fprintf(m_psIOFile, "Left ");
         SetMotor(&CDifferentialDriveController::SetLeftMotor, punLeftMotorArgs + 2);
      }

      if(punRightMotorArgs != NULL) {
         fprintf(m_psIOFile, "Right ");
         SetMotor(&CDifferentialDriveController::SetRightMotor, punRightMotorArgs + 2);
      }
   }
}


void Firmware::SetMotor(void (CDifferentialDriveController::*pf_set_motor)
                        (CDifferentialDriveController::EMode, uint8_t),
                        const char* pun_args) {
   uint16_t unNumArgs = 0, unSpeed = 0;
   char chMode;
   char pchPWMMode[3];
   bool bInputValid = false;
   if(pun_args != NULL) {
      unNumArgs = sscanf(pun_args, "%c%2[fsd]:0x%02x", &chMode, pchPWMMode, &unSpeed);
      if(unNumArgs > 0) {
         switch(chMode) {
         case 'c':
            (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::COAST,0);
            fprintf(m_psIOFile, "motor mode set to coast\r\n", pun_args);
            bInputValid = true;
            break;
         case 'b':
            (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::BRAKE,0);
            fprintf(m_psIOFile, "motor mode set to brake\r\n", pun_args);
            bInputValid = true;
            break;
         case 'f':
            if(unNumArgs == 1) {
               (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::FORWARD,0);
               fprintf(m_psIOFile, "motor mode set to forwards\r\n", pun_args);
               bInputValid = true;
            }
            else if(unNumArgs == 3) {
               if(strstr(pchPWMMode, "fd") != NULL && (unSpeed >= 0x00) && (unSpeed <= 0xFF)) {
                  (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::FORWARD_PWM_FD,
                                                            unSpeed);
                  fprintf(m_psIOFile, "motor mode set to forwards PWM fast decay at %u\r\n", unSpeed);
                  bInputValid = true;
               }
               else if(strstr(pchPWMMode, "sd") != NULL && (unSpeed >= 0x00) && (unSpeed <= 0xFF)) {
                  (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::FORWARD_PWM_SD,
                                                            unSpeed);
                  fprintf(m_psIOFile, "motor mode set to forwards PWM slow decay at %u\r\n", unSpeed);
                  bInputValid = true;
               }
            }
            break;
         case 'r':
            if(unNumArgs == 1) {
               (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::REVERSE,0);
               bInputValid = true;
               fprintf(m_psIOFile, "motor mode set to reverse\r\n", pun_args);
            }
            else if(unNumArgs == 3) {
               if(strstr(pchPWMMode, "fd") != NULL && (unSpeed >= 0x00) && (unSpeed <= 0xFF)) {
                  (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::REVERSE_PWM_FD,
                                                            unSpeed);
                  fprintf(m_psIOFile, "motor mode set to reverse PWM fast decay at %u\r\n", unSpeed);
                  bInputValid = true;
               }
               else if(strstr(pchPWMMode, "sd") != NULL && (unSpeed >= 0x00) && (unSpeed <= 0xFF)) {
                  (cDifferentialDriveController.*pf_set_motor)(CDifferentialDriveController::EMode::REVERSE_PWM_SD,
                                                            unSpeed);
                  fprintf(m_psIOFile, "motor mode set to reverse PWM slow decay at %u\r\n", unSpeed);
                  bInputValid = true;
               }
            }
            break;
         }
      }
   }
   if(!bInputValid) {
      cDifferentialDriveController.Disable();
      fprintf(m_psIOFile, INVALID_PARAM);
   }
   else {
      cDifferentialDriveController.Enable();
   }
   return;
}


void Firmware::SetLEDsMaxCurrent(const char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0xFF) {
      cLEDsCurrentController.SetActualValue(unVal);
      fprintf(m_psIOFile, "Maximum LED current set to %u\r\n", unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetMotorsMaxCurrent(const char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0xFF) {
      cMotorsCurrentController.SetActualValue(unVal);
      fprintf(m_psIOFile, "Maximum motor current set to %u\r\n", unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::SetUSBMaxCurrent(const char* pun_args) {
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
         fprintf(m_psIOFile, "Maximum USB current set to %u\r\n", unVal);
         return;
      }
   }
   fprintf(m_psIOFile, INVALID_PARAM);
}

void Firmware::SetLEDsEnable(const char* pun_args) {
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

void Firmware::SetMotorsEnable(const char* pun_args) {
   if(pun_args != NULL && strstr(pun_args, "on") != NULL) {
      DDRJ |= 0x20;
      PORTJ |= 0x20;      
      fprintf(m_psIOFile, "Motors Enable ON\r\n");
   }
   else if(pun_args != NULL && strstr(pun_args, "off") != NULL) {
      DDRJ |= 0x20;
      PORTJ &= ~0x20;
      fprintf(m_psIOFile, "Motors Enable OFF\r\n");
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}


void Firmware::SetSystemEnable(const char* pun_args) {
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

void Firmware::SetBQ24250VDPMTo4V2(const char* pun_args) {
   cBQ24250Controller.SetRegisterValue(0x04, R4_VDPM_MASK, 0);
   fprintf(m_psIOFile, "BQ24250 VDPM set to 4.2V\r\n");
}

void Firmware::SetBQ24250InputEnable(const char* pun_args) {
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


void Firmware::SetBQ24161InputCurrent(const char* pun_args) {
   bool bInputValid = false;
   uint16_t unVal;
   if(pun_args != NULL) {
      sscanf(pun_args, "%u", &unVal);
      bInputValid = true;
      switch(unVal) {
      case 100:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L100);
         break;
      case 150:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L150);
         break;
      case 500:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L500);
         break;
      case 800:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L900);
         break;
      case 900:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L900);
         break;
      case 1500:
         cBQ24161Controller.SetUSBInputLimit(CBQ24161Controller::EUSBInputLimit::L1500);
         break;
      default:
         bInputValid = false;
         break;
      }
   }
   /* report if the input was valid */
   if(bInputValid) {
      fprintf(m_psIOFile, "BQ24250 Input Current %s\r\n", pun_args);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}


void Firmware::SetBQ24250InputCurrent(const char* pun_args) {
   bool bInputValid = false;
   uint16_t unVal;
   if(pun_args != NULL) {
      if(strstr(pun_args, "HIZ") != NULL) {
         cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::LHIZ);
         bInputValid = true;
      }
      else {
         sscanf(pun_args, "%u", &unVal);
         bInputValid = true;
         switch(unVal) {
         case 100:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L100);
            break;
         case 150:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L150);
            break;
         case 500:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L500);
            break;
         case 900:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L900);
            break;
         case 1500:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L1500);
            break;
         case 2000:
            cBQ24250Controller.SetInputCurrentLimit(CBQ24250Controller::EInputCurrentLimit::L2000);
            break;
         default:
            bInputValid = false;
            break;
         }
      }
   }
   /* report if the input was valid */
   if(bInputValid) {
      fprintf(m_psIOFile, "BQ24250 Input Current %s\r\n", pun_args);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::GetBQ24250Register(const char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0x07) {
      cBQ24250Controller.DumpRegister(unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}

void Firmware::GetBQ24161Register(const char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "0x%x", &unVal) == 1 && unVal >= 0x00 && unVal <= 0x07) {
      cBQ24161Controller.DumpRegister(unVal);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}


void Firmware::TestPMIC(const char* pun_args) {
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

void Firmware::CheckFaults(const char* pun_args) {
   fprintf(m_psIOFile, "LED Fault: %c\r\n", (PINJ & 0x04)==0?'T':'F');
   fprintf(m_psIOFile, "MTR Fault: %c\r\n", (PINJ & 0x10)==0?'T':'F');
   fprintf(m_psIOFile, "USB Fault: %c\r\n", (PINJ & 0x01)==0?'T':'F');
}

void Firmware::SetWatchdogPeriod(const char* pun_args) {
   uint16_t unVal = 0;
   if(pun_args != NULL && sscanf(pun_args, "%u", &unVal) == 1) {
      unWatchdogPeriod = unVal;
      fprintf(m_psIOFile, "Watchdog period set to %u\r\n", unWatchdogPeriod);
   }
   else {
      fprintf(m_psIOFile, INVALID_PARAM);
   }
}
