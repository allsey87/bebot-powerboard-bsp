#include "firmware.h"

/* initialisation of the static singleton */
Firmware Firmware::_firmware;

/* main function that runs the firmware */
int main(void)
{
   /* FILE structs for fprintf */
   FILE huart;

   /* Set up FILE structs for fprintf */                           
   fdev_setup_stream(&huart, 
                     [](char c_to_write, FILE* pf_stream) {
                        Firmware::GetInstance().GetHUARTController().Write(c_to_write);
                        return 1;
                     },
                     [](FILE* pf_stream) {
                        return int(Firmware::GetInstance().GetHUARTController().Read());
                     },
                     _FDEV_SETUP_RW);

   Firmware::GetInstance().SetFilePointer(&huart);

   /* Execute the firmware */
   return Firmware::GetInstance().Exec();
}

/***********************************************************/
/***********************************************************/

int Firmware::Exec() 
{
   uint8_t unInput = 0;

   /* Set default values */
   PORTD &= ~(PIN_ACTUATORS_EN | PIN_BQ24250_INPUT_EN | PIN_VUSB50_L500_EN);
   PORTD |= (PIN_SYSTEM_EN);
   /* set as outputs */
   DDRD |= (PIN_ACTUATORS_EN | PIN_SYSTEM_EN | PIN_BQ24250_INPUT_EN | PIN_VUSB50_L500_EN); 
      
   fprintf(m_psHUART, "Ready> ");

   for(;;) {
      if(Firmware::GetInstance().GetHUARTController().Available()) {
         unInput = Firmware::GetInstance().GetHUARTController().Read();
         /* flush */
         while(Firmware::GetInstance().GetHUARTController().Available()) {
            Firmware::GetInstance().GetHUARTController().Read();
         }
      }
      else {
         unInput = 0;
      }

      if(unInput != 0) {
         fprintf(m_psHUART, "\r\n");
         switch(unInput) {
         case 'u':
            fprintf(m_psHUART, "Uptime = %lums\r\n", m_cTimer.GetMilliseconds());
            break;
         case 'p':
            TestPMICs();
            break;
         case 'd':
            fprintf(m_psHUART, "BQ24161\r\n");
            for(uint8_t i = 0; i < 2; i++) {
               cBQ24161Controller.DumpRegister(i);
            }
            fprintf(m_psHUART, "BQ24250\r\n");
            for(uint8_t i = 0; i < 2; i++) {
               cBQ24250Controller.DumpRegister(i);
            }
            break;
         case 'a':
            if(PORTD & PIN_ACTUATORS_EN) {
               PORTD &= ~PIN_ACTUATORS_EN;
               fprintf(m_psHUART, "Actuators switched off\r\n");
            }
            else {
               PORTD |= PIN_ACTUATORS_EN;
               fprintf(m_psHUART, "Actuators switched on\r\n");
            }
            break;
         case 's':
            if(PORTD & PIN_SYSTEM_EN) {
               PORTD &= ~PIN_SYSTEM_EN;
               fprintf(m_psHUART, "System switched off\r\n");
            }
            else {
               PORTD |= PIN_SYSTEM_EN;
               fprintf(m_psHUART, "System switched on\r\n");
            }
            break;
         case 'i':
            if(PORTD & PIN_BQ24250_INPUT_EN) {
               PORTD &= ~PIN_BQ24250_INPUT_EN;
               fprintf(m_psHUART, "BQ24250 input off\r\n");
            }
            else {
               PORTD |= PIN_BQ24250_INPUT_EN;
               fprintf(m_psHUART, "BQ24250 input on\r\n");
            }
            break;
         default:
            break;
         }
         fprintf(m_psHUART, "Ready> ");
      }
   }
   return 0;
}

/***********************************************************/
/***********************************************************/

void Firmware::TestPMICs() {
   cBQ24161Controller.Synchronize();
   fprintf(m_psHUART, "<BQ24161>");
   fprintf(m_psHUART, "\r\nstate: ");
   switch(cBQ24161Controller.GetDeviceState()) {
   case CBQ24161Controller::EDeviceState::STANDBY:
      fprintf(m_psHUART, "standby");
      break;
   case CBQ24161Controller::EDeviceState::READY:
      fprintf(m_psHUART, "ready");
      break;
   case CBQ24161Controller::EDeviceState::CHARGING:
      fprintf(m_psHUART, "charging");
      break;
   case CBQ24161Controller::EDeviceState::DONE:
      fprintf(m_psHUART, "done");
      break;
   case CBQ24161Controller::EDeviceState::FAULT:
      fprintf(m_psHUART, "fault");
      break;
   }
   fprintf(m_psHUART, "\r\nfault: ");
   switch(cBQ24161Controller.GetFault()) {
   case CBQ24161Controller::EFault::NONE:
      fprintf(m_psHUART, "none");
      break;
   case CBQ24161Controller::EFault::DEV_THERMAL_SHDN:
      fprintf(m_psHUART, "dev_thermal_shdn");
      break;
   case CBQ24161Controller::EFault::BATT_THERMAL_SHDN:
      fprintf(m_psHUART, "batt_thermal_shdn");
      break;
   case CBQ24161Controller::EFault::WATCHDOG_TMR_EXPR:
      fprintf(m_psHUART, "watchdog_tmr_expr");
      break;
   case CBQ24161Controller::EFault::SAFETY_TMR_EXPR:
      fprintf(m_psHUART, "safety_tmr_expr");
      break;
   case CBQ24161Controller::EFault::ADAPTER_FAULT:
      fprintf(m_psHUART, "adapter_fault");
      break;
   case CBQ24161Controller::EFault::USB_FAULT:
      fprintf(m_psHUART, "usb_fault");
      break;
   case CBQ24161Controller::EFault::BATT_FAULT:
      fprintf(m_psHUART, "batt_fault");
      break;
   }           
   fprintf(m_psHUART, "\r\nselected source: ");
   switch(cBQ24161Controller.GetSelectedSource()) {
   case CBQ24161Controller::ESource::NONE:
      fprintf(m_psHUART, "none");
      break;
   case CBQ24161Controller::ESource::ADAPTER:
      fprintf(m_psHUART, "adapter");
      break;
   case CBQ24161Controller::ESource::USB:
      fprintf(m_psHUART, "usb");
      break;
   }
   fprintf(m_psHUART, "\r\nadapter input: ");
   switch(cBQ24161Controller.GetAdapterInputState()) {
   case CBQ24161Controller::EInputState::NORMAL:
      fprintf(m_psHUART, "normal");
      break;
   case CBQ24161Controller::EInputState::OVER_VOLTAGE:
      fprintf(m_psHUART, "over_voltage");
      break;
   case CBQ24161Controller::EInputState::WEAK_SOURCE:
      fprintf(m_psHUART, "weak_source");
      break;
   case CBQ24161Controller::EInputState::UNDER_VOLTAGE:
      fprintf(m_psHUART, "under_voltage");
      break;
   }
   fprintf(m_psHUART, "\r\nusb input: ");
   switch(cBQ24161Controller.GetUSBInputState()) {
   case CBQ24161Controller::EInputState::NORMAL:
      fprintf(m_psHUART, "normal");
      break;
   case CBQ24161Controller::EInputState::OVER_VOLTAGE:
      fprintf(m_psHUART, "over_voltage");
      break;
   case CBQ24161Controller::EInputState::WEAK_SOURCE:
      fprintf(m_psHUART, "weak_source");
      break;
   case CBQ24161Controller::EInputState::UNDER_VOLTAGE:
      fprintf(m_psHUART, "under_voltage");
      break;
   }
   fprintf(m_psHUART, "\r\nbattery_state: ");
   switch(cBQ24161Controller.GetBatteryState()) {
   case CBQ24161Controller::EBatteryState::NORMAL:
      fprintf(m_psHUART, "normal");
      break;
   case CBQ24161Controller::EBatteryState::OVER_VOLTAGE:
      fprintf(m_psHUART, "over_voltage");
      break;
   case CBQ24161Controller::EBatteryState::DISCONNECTED:
      fprintf(m_psHUART, "disconnected");
      break;
   case CBQ24161Controller::EBatteryState::UNDEFINED:
      fprintf(m_psHUART, "undefined");
      break;
   }
   fprintf(m_psHUART, "\r\n");
   cBQ24250Controller.Synchronize();
   fprintf(m_psHUART, "<BQ24250>\r\n");
   fprintf(m_psHUART, "state: ");
   switch(cBQ24250Controller.GetDeviceState()) {
   case CBQ24250Controller::EDeviceState::READY:
      fprintf(m_psHUART, "ready");
      break;
   case CBQ24250Controller::EDeviceState::CHARGING:
      fprintf(m_psHUART, "charging");
      break;
   case CBQ24250Controller::EDeviceState::DONE:
      fprintf(m_psHUART, "done");
      break;
   case CBQ24250Controller::EDeviceState::FAULT:
      fprintf(m_psHUART, "fault");
      break;
   }
   fprintf(m_psHUART, "\r\nfault: ");
   switch(cBQ24250Controller.GetFault()) {
   case CBQ24250Controller::EFault::NONE:
      fprintf(m_psHUART, "none");
      break;
   case CBQ24250Controller::EFault::INPUT_OVER_VOLTAGE:
      fprintf(m_psHUART, "input_over_voltage");
      break;
   case CBQ24250Controller::EFault::INPUT_UNDER_VOLTAGE:
      fprintf(m_psHUART, "input_under_voltage");
      break;
   case CBQ24250Controller::EFault::SLEEP:
      fprintf(m_psHUART, "sleep");
      break;
   case CBQ24250Controller::EFault::BATT_THERMAL_SHDN:
      fprintf(m_psHUART, "batt_thermal_shdn");
      break;
   case CBQ24250Controller::EFault::BATT_OVER_VOLTAGE:
      fprintf(m_psHUART, "batt_over_voltage");
      break;
   case CBQ24250Controller::EFault::DEV_THERMAL_SHDN:
      fprintf(m_psHUART, "dev_thermal_shdn");
      break;
   case CBQ24250Controller::EFault::DEV_TIMER_FAULT:
      fprintf(m_psHUART, "dev_timer_fault");
      break;
   case CBQ24250Controller::EFault::BATT_DISCONNECTED:
      fprintf(m_psHUART, "batt_disconnected");
      break;
   case CBQ24250Controller::EFault::ISET_SHORTED:
      fprintf(m_psHUART, "iset_shorted");
      break;
   case CBQ24250Controller::EFault::INPUT_FAULT:
      fprintf(m_psHUART, "input_fault");
      break;
   case CBQ24250Controller::EFault::UNDEFINED:
      fprintf(m_psHUART, "undefined");
      break;
   }
   fprintf(m_psHUART, "\r\nwatchdog enabled = %c", cBQ24250Controller.GetWatchdogEnabled()?'t':'f');
   fprintf(m_psHUART, "\r\nwatchdog fault = %c", cBQ24250Controller.GetWatchdogFault()?'t':'f');

   fprintf(m_psHUART, "\r\n");
}
