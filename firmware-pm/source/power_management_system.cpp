
#include "power_management_system.h"

#include <firmware.h>

/* Port D control pins */
#define PIN_ACTUATORS_EN 0x80
#define PIN_SYSTEM_EN 0x10
#define PIN_PASSTHROUGH_EN 0x40
#define PIN_VUSB50_L500_EN 0x20

/* Coefficient for converting ADC measurement to battery voltage.
   assumes a 1V1 reference and a 1M/330k voltage divider */
#define ADC_BATT_MV_COEFF 17

/* TW addresses of configurable devices */
#define INPUT_STATUS_LEDS_ADDR 0x60
#define BATT_STATUS_LEDS_ADDR 0x61     

/* Definitions for status LEDs */
#define ADP_LED_INDEX 0
#define USB_LP_LED_INDEX 1
#define USB_FP_LED_INDEX 2
#define USB_HP_LED_INDEX 3

#define BATT1_STAT_INDEX 0
#define BATT1_CHRG_INDEX 1
#define BATT2_STAT_INDEX 2
#define BATT2_CHRG_INDEX 3

/***********************************************************/
/***********************************************************/

CPowerManagementSystem::CPowerManagementSystem() :
   m_cBatteryStatusLEDs(BATT_STATUS_LEDS_ADDR),
   m_cInputStatusLEDs(INPUT_STATUS_LEDS_ADDR) {
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::Init() {

   SetSystemToActuatorPassthroughPowerOn(false);
   SetActuatorPowerOn(false);
   
   m_cActuatorPowerManager.SetInputCurrentLimit(CBQ24250Module::EInputCurrentLimit::LHIZ);

   m_cSystemPowerManager.SetBatteryRegulationVoltage(SYS_BATT_REG_VOLTAGE);
   m_cSystemPowerManager.SetBatteryChargingCurrent(SYS_BATT_CHG_CURRENT);
   m_cSystemPowerManager.SetBatteryTerminationCurrent(SYS_BATT_TRM_CURRENT);

   CPCA9633Module::ResetDevices();
   m_cInputStatusLEDs.Init();
   m_cBatteryStatusLEDs.Init();

   Update();
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::SetSystemPowerOn(bool b_set_power_on) {
   if(b_set_power_on)
      PORTD |= PIN_SYSTEM_EN;
   else
      PORTD &= ~PIN_SYSTEM_EN;
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::SetActuatorPowerOn(bool b_set_power_on) {
   if(b_set_power_on)
      PORTD |= PIN_ACTUATORS_EN;
   else
      PORTD &= ~PIN_ACTUATORS_EN;
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::SetSystemToActuatorPassthroughPowerOn(bool b_set_power_on) {
   if(b_set_power_on)
      PORTD |= PIN_PASSTHROUGH_EN;
   else
      PORTD &= ~PIN_PASSTHROUGH_EN;
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::Update() {
   /* Reset watchdogs and synchronise state with remote PMICs */
   m_cSystemPowerManager.ResetWatchdogTimer();
   m_cSystemPowerManager.Synchronize();
   m_cActuatorPowerManager.ResetWatchdogTimer();
   m_cActuatorPowerManager.Synchronize();

   /* Read battery voltages */
   m_unSystemBatteryVoltage = 
      m_cADCController.GetValue(CADCController::EChannel::ADC6) * ADC_BATT_MV_COEFF;
   m_unActuatorBatteryVoltage = 
      m_cADCController.GetValue(CADCController::EChannel::ADC7) * ADC_BATT_MV_COEFF;

   /* Configure status LEDs */
   if(m_cSystemPowerManager.GetBatteryState() != CBQ24161Module::EBatteryState::NORMAL ||
      m_cSystemPowerManager.GetFault() == CBQ24161Module::EFault::BATT_FAULT ||
      m_cSystemPowerManager.GetFault() == CBQ24161Module::EFault::BATT_THERMAL_SHDN) {
            
      m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::BLINK);
      m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::OFF);
   }

   //if both batteries charged, or no source connected AND system powered off, enter low power mode
}

/***********************************************************/
/***********************************************************/

bool CPowerManagementSystem::IsUSBConnected() {
   return m_cSystemPowerManager.GetUSBInputState() == 
      CBQ24161Module::EInputState::NORMAL;
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::PrintStatus() {
   /* System power manager */
   m_cSystemPowerManager.Synchronize();
   fprintf(CFirmware::GetInstance().m_psHUART, "<System PMIC>\r\n");
   for(uint8_t i = 0; i < 7; i++) {
      m_cSystemPowerManager.DumpRegister(i);
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "---\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "state: ");
   switch(m_cSystemPowerManager.GetDeviceState()) {
   case CBQ24161Module::EDeviceState::STANDBY:
      fprintf(CFirmware::GetInstance().m_psHUART, "standby");
      break;
   case CBQ24161Module::EDeviceState::READY:
      fprintf(CFirmware::GetInstance().m_psHUART, "ready");
      break;
   case CBQ24161Module::EDeviceState::CHARGING:
      fprintf(CFirmware::GetInstance().m_psHUART, "charging");
      break;
   case CBQ24161Module::EDeviceState::DONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "done");
      break;
   case CBQ24161Module::EDeviceState::FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "fault");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "fault: ");
   switch(m_cSystemPowerManager.GetFault()) {
   case CBQ24161Module::EFault::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "none");
      break;
   case CBQ24161Module::EFault::DEV_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "dev_thermal_shdn");
      break;
   case CBQ24161Module::EFault::BATT_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "batt_thermal_shdn");
      break;
   case CBQ24161Module::EFault::WATCHDOG_TMR_EXPR:
      fprintf(CFirmware::GetInstance().m_psHUART, "watchdog_tmr_expr");
      break;
   case CBQ24161Module::EFault::SAFETY_TMR_EXPR:
      fprintf(CFirmware::GetInstance().m_psHUART, "safety_tmr_expr");
      break;
   case CBQ24161Module::EFault::ADAPTER_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "adapter_fault");
      break;
   case CBQ24161Module::EFault::USB_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "usb_fault");
      break;
   case CBQ24161Module::EFault::BATT_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "batt_fault");
      break;
   }           
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "selected source: ");
   switch(m_cSystemPowerManager.GetSelectedSource()) {
   case CBQ24161Module::ESource::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "none");
      break;
   case CBQ24161Module::ESource::ADAPTER:
      fprintf(CFirmware::GetInstance().m_psHUART, "adapter");
      break;
   case CBQ24161Module::ESource::USB:
      fprintf(CFirmware::GetInstance().m_psHUART, "usb");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "adapter input: ");
   switch(m_cSystemPowerManager.GetAdapterInputState()) {
   case CBQ24161Module::EInputState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "normal");
      break;
   case CBQ24161Module::EInputState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "over_voltage");
      break;
   case CBQ24161Module::EInputState::WEAK_SOURCE:
      fprintf(CFirmware::GetInstance().m_psHUART, "weak_source");
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "under_voltage");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\nusb input: ");
   switch(m_cSystemPowerManager.GetUSBInputState()) {
   case CBQ24161Module::EInputState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "normal");
      break;
   case CBQ24161Module::EInputState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "over_voltage");
      break;
   case CBQ24161Module::EInputState::WEAK_SOURCE:
      fprintf(CFirmware::GetInstance().m_psHUART, "weak_source");
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "under_voltage");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "battery_state: ");
   switch(m_cSystemPowerManager.GetBatteryState()) {
   case CBQ24161Module::EBatteryState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "normal");
      break;
   case CBQ24161Module::EBatteryState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "over_voltage");
      break;
   case CBQ24161Module::EBatteryState::DISCONNECTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "disconnected");
      break;
   case CBQ24161Module::EBatteryState::UNDEFINED:
      fprintf(CFirmware::GetInstance().m_psHUART, "undefined");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   /* Actuator power manager */
   m_cActuatorPowerManager.Synchronize();
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "<Actuator PMIC>\r\n");
   for(uint8_t i = 0; i < 2; i++) {
      m_cActuatorPowerManager.DumpRegister(i);
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "---\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "state: ");
   switch(m_cActuatorPowerManager.GetDeviceState()) {
   case CBQ24250Module::EDeviceState::READY:
      fprintf(CFirmware::GetInstance().m_psHUART, "ready");
      break;
   case CBQ24250Module::EDeviceState::CHARGING:
      fprintf(CFirmware::GetInstance().m_psHUART, "charging");
      break;
   case CBQ24250Module::EDeviceState::DONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "done");
      break;
   case CBQ24250Module::EDeviceState::FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "fault");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "fault: ");
   switch(m_cActuatorPowerManager.GetFault()) {
   case CBQ24250Module::EFault::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "none");
      break;
   case CBQ24250Module::EFault::INPUT_OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "input_over_voltage");
      break;
   case CBQ24250Module::EFault::INPUT_UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "input_under_voltage");
      break;
   case CBQ24250Module::EFault::SLEEP:
      fprintf(CFirmware::GetInstance().m_psHUART, "sleep");
      break;
   case CBQ24250Module::EFault::BATT_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "batt_thermal_shdn");
      break;
   case CBQ24250Module::EFault::BATT_OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "batt_over_voltage");
      break;
   case CBQ24250Module::EFault::DEV_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "dev_thermal_shdn");
      break;
   case CBQ24250Module::EFault::DEV_TIMER_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "dev_timer_fault");
      break;
   case CBQ24250Module::EFault::BATT_DISCONNECTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "batt_disconnected");
      break;
   case CBQ24250Module::EFault::ISET_SHORTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "iset_shorted");
      break;
   case CBQ24250Module::EFault::INPUT_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "input_fault");
      break;
   case CBQ24250Module::EFault::UNDEFINED:
      fprintf(CFirmware::GetInstance().m_psHUART, "undefined");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, 
           "watchdog enabled = %c\r\n", 
           m_cActuatorPowerManager.GetWatchdogEnabled()?'t':'f');
   fprintf(CFirmware::GetInstance().m_psHUART, 
           "watchdog fault = %c\r\n", 
           m_cActuatorPowerManager.GetWatchdogFault()?'t':'f');
}


