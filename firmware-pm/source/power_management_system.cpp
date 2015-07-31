
#include "power_management_system.h"

#include <firmware.h>

/* Port D control pins */
#define PIN_ACTUATORS_EN 0x80
#define PIN_SYSTEM_EN 0x10
#define PIN_PASSTHROUGH_EN 0x40
#define PIN_VUSB50_L500_EN 0x20

/* Coefficient for converting ADC measurement to battery voltage.
   assumes a 1V1 reference and a 1M/330k voltage divider */
#define ADC_BATT_MV_COEFF 17u

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

/* Power requirements for various parts of the system */
#define SYS_POWER_REQ 750
#define ACT_POWER_REQ 1500

/* Battery parameters */
#define SYS_BATT_REG_VOLTAGE 4200
#define SYS_BATT_INIT_CHG_VOLTAGE 4000
#define SYS_BATT_CHG_CURRENT 740
#define SYS_BATT_TRM_CURRENT 50
#define SYS_BATT_LOW_VOLTAGE 3200

#define ACT_BATT_REG_VOLTAGE 4200
#define ACT_BATT_INIT_CHG_VOLTAGE 4000
#define ACT_BATT_CHG_CURRENT 370
#define ACT_BATT_TRM_CURRENT 50
#define ACT_BATT_LOW_VOLTAGE 3200

/***********************************************************/
/***********************************************************/

CPowerManagementSystem::CPowerManagementSystem() :
   m_cBatteryStatusLEDs(BATT_STATUS_LEDS_ADDR),
   m_cInputStatusLEDs(INPUT_STATUS_LEDS_ADDR) {
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::Init() {
   /* Init base power configuration */
   SetSystemToActuatorPassthroughPowerOn(false);
   SetActuatorPowerOn(false);
   SetSystemPowerOn(true);

   /* Init output control pins - this overrides hardware pull ups / downs */
   DDRD |= (PIN_ACTUATORS_EN | PIN_SYSTEM_EN | PIN_PASSTHROUGH_EN);
   
   m_cActuatorPowerManager.SetInputLimit(CBQ24250Module::EInputLimit::LHIZ);

   m_cSystemPowerManager.SetChargingEnable(false);
   m_cActuatorPowerManager.SetChargingEnable(false);

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

bool CPowerManagementSystem::IsSystemPowerOn() {
   return ((PORTD & PIN_SYSTEM_EN) != 0);
}

/***********************************************************/
/***********************************************************/

bool CPowerManagementSystem::IsActuatorPowerOn() {
   return ((PORTD & PIN_ACTUATORS_EN) != 0);
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::Update() {
   /* Reset watchdogs and synchronise state with remote PMICs */
   m_cSystemPowerManager.ResetWatchdogTimer();
   m_cSystemPowerManager.Synchronize();
   m_cActuatorPowerManager.ResetWatchdogTimer();
   m_cActuatorPowerManager.Synchronize();

   /* Reflect the state of the system power sources on the LEDs */
   /* Adapter */
   switch(m_cSystemPowerManager.GetInputState(CBQ24161Module::ESource::ADAPTER)) {
   case CBQ24161Module::EInputState::NORMAL:
      m_cInputStatusLEDs.SetLEDMode(ADP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      m_cInputStatusLEDs.SetLEDMode(ADP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
      break;
   default:
      m_cInputStatusLEDs.SetLEDMode(ADP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
      break;
   }
   /* USB */
   switch(m_cSystemPowerManager.GetInputState(CBQ24161Module::ESource::USB)) {
   case CBQ24161Module::EInputState::NORMAL:
      switch(m_cSystemPowerManager.GetInputLimit(CBQ24161Module::ESource::USB)) {
      case CBQ24161Module::EInputLimit::L100:
      case CBQ24161Module::EInputLimit::L150:
         m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
         m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
         break;
      case CBQ24161Module::EInputLimit::L500:
         m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
         break;
      case CBQ24161Module::EInputLimit::L800:
      case CBQ24161Module::EInputLimit::L900:
      case CBQ24161Module::EInputLimit::L1500:
         m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::ON);
         break;
      default:
         /* Indicate error condition */
         m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
         m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
         m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
         break;
      }
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
      m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
      m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::OFF);
      break;
   default:
      /* Indicate error condition */
      m_cInputStatusLEDs.SetLEDMode(USB_LP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
      m_cInputStatusLEDs.SetLEDMode(USB_HP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
      m_cInputStatusLEDs.SetLEDMode(USB_FP_LED_INDEX, CPCA9633Module::ELEDMode::BLINK);
      break;
   }

   /* Determine power available to the system */
   uint16_t unAvailableCurrent = 0;
   /* create an ordered list of sources to be checked */
   CBQ24161Module::ESource peInputSourceList[3];

   switch(m_cSystemPowerManager.GetPreferredSource()) {
   case CBQ24161Module::ESource::ADAPTER:
      peInputSourceList[0] = CBQ24161Module::ESource::ADAPTER;
      peInputSourceList[1] = CBQ24161Module::ESource::USB;
      peInputSourceList[2] = CBQ24161Module::ESource::NONE;
      break;
   case CBQ24161Module::ESource::USB:
      peInputSourceList[0] = CBQ24161Module::ESource::USB;
      peInputSourceList[1] = CBQ24161Module::ESource::ADAPTER;
      peInputSourceList[2] = CBQ24161Module::ESource::NONE;
      break;
   default:
      peInputSourceList[0] = CBQ24161Module::ESource::NONE;
      peInputSourceList[1] = CBQ24161Module::ESource::NONE;
      peInputSourceList[2] = CBQ24161Module::ESource::NONE;
      break;
   }
   /* for each source check the input state and limit */
   for(CBQ24161Module::ESource eInputSource : peInputSourceList) {
      if(m_cSystemPowerManager.GetInputState(eInputSource) != CBQ24161Module::EInputState::NORMAL) {
         continue;
      }
      else {
         switch(m_cSystemPowerManager.GetInputLimit(eInputSource)) {
         case CBQ24161Module::EInputLimit::L0:
            unAvailableCurrent = 0;
            break;
         case CBQ24161Module::EInputLimit::L100:
            unAvailableCurrent = 100;
            break;
         case CBQ24161Module::EInputLimit::L150:
            unAvailableCurrent = 150;
            break;
         case CBQ24161Module::EInputLimit::L500:
            unAvailableCurrent = 500;
            break;
         case CBQ24161Module::EInputLimit::L800:
            unAvailableCurrent = 800;
            break;
         case CBQ24161Module::EInputLimit::L900:
            unAvailableCurrent = 900;
            break;
         case CBQ24161Module::EInputLimit::L1500:
            unAvailableCurrent = 1500;
            break;
         case CBQ24161Module::EInputLimit::L2500:
            unAvailableCurrent = 2500;
            break;
         }
         /* At this point we have a valid input source selected, break the loop */
         if(unAvailableCurrent > 0) break;
      }
   }

   /* Read battery voltages */
   m_unSystemBatteryVoltage = 
      m_cADCController.GetValue(CADCController::EChannel::ADC6) * ADC_BATT_MV_COEFF;
   m_unActuatorBatteryVoltage = 
      m_cADCController.GetValue(CADCController::EChannel::ADC7) * ADC_BATT_MV_COEFF;

   /* Allocate power to the system if switched on */
   if(IsSystemPowerOn()) {
      if(unAvailableCurrent > SYS_POWER_REQ) {
         unAvailableCurrent -= SYS_POWER_REQ;
      }
   }

   /* Enable charging the system battery if at least half of the charge current 
      is available. BQ24161 automatically prioritizes system power. */
   if(m_cSystemPowerManager.GetBatteryState() != CBQ24161Module::EBatteryState::NORMAL ||
      m_cSystemPowerManager.GetFault() == CBQ24161Module::EFault::BATT_FAULT ||
      m_cSystemPowerManager.GetFault() == CBQ24161Module::EFault::BATT_THERMAL_SHDN) {
      /* There is an issue with the battery */
      m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::BLINK);
      m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::OFF);    
   }
   else {
      /* If the battery is above SYS_BATT_INIT_CHG_VOLTAGE */
      if(m_unSystemBatteryVoltage > SYS_BATT_INIT_CHG_VOLTAGE) {
         /* Battery is charged or being charged */
         if(m_cSystemPowerManager.GetDeviceState() == CBQ24161Module::EDeviceState::CHARGING) {
            /* Terminate charging if available current drops below a third the charge current */
            if(unAvailableCurrent < (SYS_BATT_CHG_CURRENT / 3)) {             
               m_cSystemPowerManager.SetChargingEnable(false);
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::ON);
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::ON);
            }
            else {
               /* Continue charging */
               unAvailableCurrent -= (SYS_BATT_CHG_CURRENT / 3);
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::BLINK);
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::ON);
            }
         }
         else { /* m_cSystemPowerManager.GetDeviceState() != CBQ24161::EDeviceState::CHARGING */
            m_cSystemPowerManager.SetChargingEnable(false);
            /* system battery voltage is high enough to be considered charged */
            m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::ON);
            m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::ON);
         }
      }
      else { /* m_unSystemBatteryVoltage <= SYS_BATT_INIT_CHG_VOLTAGE */
         if(unAvailableCurrent > (SYS_BATT_CHG_CURRENT / 3)) {
            unAvailableCurrent -= (SYS_BATT_CHG_CURRENT / 3);
            m_cSystemPowerManager.SetChargingEnable(true);
            m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::BLINK);
            m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::ON);
         }
         else {
            /* Battery is discharging and could be charged */
            m_cBatteryStatusLEDs.SetLEDMode(BATT1_CHRG_INDEX, CPCA9633Module::ELEDMode::OFF);
            if(m_unSystemBatteryVoltage < SYS_BATT_LOW_VOLTAGE) {
               /* Low battery warning */
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::BLINK);
            }
            else {
               m_cBatteryStatusLEDs.SetLEDMode(BATT1_STAT_INDEX, CPCA9633Module::ELEDMode::ON);
            }
         } 
      }
   }

   /* Depending on the remaining current, enable the system to actuator power passthrough */
   CBQ24250Module::EInputLimit eInputLimit = CBQ24250Module::EInputLimit::LHIZ;

   if(unAvailableCurrent > 2250) {
      eInputLimit = CBQ24250Module::EInputLimit::L2000;
   } else if(unAvailableCurrent > 1750) {
      eInputLimit = CBQ24250Module::EInputLimit::L1500;
   } else if(unAvailableCurrent > 1150) {
      eInputLimit = CBQ24250Module::EInputLimit::L900;
   } else if(unAvailableCurrent > 750) {
      eInputLimit = CBQ24250Module::EInputLimit::L500;
   } else if(unAvailableCurrent > 300) {
      eInputLimit = CBQ24250Module::EInputLimit::L500;
   }

   if(eInputLimit == CBQ24250Module::EInputLimit::LHIZ);

   /* Allocate power to the actuators if switched on */
   if(IsActuatorPowerOn()) {
      if(unAvailableCurrent > ACT_POWER_REQ) {
         unAvailableCurrent -= ACT_POWER_REQ;
         m_cActuatorPowerManager.SetInputLimit(CBQ24250Module::EInputLimit::L1500);
         //SetSystemToActuatorPassthroughPowerOn(true);
      }
   }


   /* Configure status LEDs */
            
   //if both batteries charged, or no source connected AND system powered off, enter low power mode
}

/***********************************************************/
/***********************************************************/

bool CPowerManagementSystem::IsUSBConnected() {
   return m_cSystemPowerManager.GetInputState(CBQ24161Module::ESource::USB) == 
      CBQ24161Module::EInputState::NORMAL;
}

/***********************************************************/
/***********************************************************/

void CPowerManagementSystem::PrintStatus() {
   fprintf(CFirmware::GetInstance().m_psHUART, "Batt 1: %u\r\n",
           m_cADCController.GetValue(CADCController::EChannel::ADC6) * ADC_BATT_MV_COEFF);
   fprintf(CFirmware::GetInstance().m_psHUART, "Batt 2: %u\r\n",
           m_cADCController.GetValue(CADCController::EChannel::ADC7) * ADC_BATT_MV_COEFF);

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
      fprintf(CFirmware::GetInstance().m_psHUART, "STBY");
      break;
   case CBQ24161Module::EDeviceState::READY:
      fprintf(CFirmware::GetInstance().m_psHUART, "RDY");
      break;
   case CBQ24161Module::EDeviceState::CHARGING:
      fprintf(CFirmware::GetInstance().m_psHUART, "CHG");
      break;
   case CBQ24161Module::EDeviceState::DONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "DONE");
      break;
   case CBQ24161Module::EDeviceState::FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "FAULT");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "fault: ");
   switch(m_cSystemPowerManager.GetFault()) {
   case CBQ24161Module::EFault::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "NA");
      break;
   case CBQ24161Module::EFault::DEV_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "DTHM_SHDN");
      break;
   case CBQ24161Module::EFault::BATT_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "BTHM_SHDN");
      break;
   case CBQ24161Module::EFault::WATCHDOG_TMR_EXPR:
      fprintf(CFirmware::GetInstance().m_psHUART, "WTMR_EXP");
      break;
   case CBQ24161Module::EFault::SAFETY_TMR_EXPR:
      fprintf(CFirmware::GetInstance().m_psHUART, "STMR_EXP");
      break;
   case CBQ24161Module::EFault::ADAPTER_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "ADP");
      break;
   case CBQ24161Module::EFault::USB_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "USB");
      break;
   case CBQ24161Module::EFault::BATT_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "BATT");
      break;
   }           
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "selected source: ");
   switch(m_cSystemPowerManager.GetSelectedSource()) {
   case CBQ24161Module::ESource::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "NA");
      break;
   case CBQ24161Module::ESource::ADAPTER:
      fprintf(CFirmware::GetInstance().m_psHUART, "ADP");
      break;
   case CBQ24161Module::ESource::USB:
      fprintf(CFirmware::GetInstance().m_psHUART, "USB");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "ADP input: ");
   switch(m_cSystemPowerManager.GetInputState(CBQ24161Module::ESource::ADAPTER)) {
   case CBQ24161Module::EInputState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "OK");
      break;
   case CBQ24161Module::EInputState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "OV");
      break;
   case CBQ24161Module::EInputState::WEAK_SOURCE:
      fprintf(CFirmware::GetInstance().m_psHUART, "WS");
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "UV");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\nusb input: ");
   switch(m_cSystemPowerManager.GetInputState(CBQ24161Module::ESource::USB)) {
   case CBQ24161Module::EInputState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "OK");
      break;
   case CBQ24161Module::EInputState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "OV");
      break;
   case CBQ24161Module::EInputState::WEAK_SOURCE:
      fprintf(CFirmware::GetInstance().m_psHUART, "WS");
      break;
   case CBQ24161Module::EInputState::UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "UV");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "battery_state: ");
   switch(m_cSystemPowerManager.GetBatteryState()) {
   case CBQ24161Module::EBatteryState::NORMAL:
      fprintf(CFirmware::GetInstance().m_psHUART, "OK");
      break;
   case CBQ24161Module::EBatteryState::OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "OV");
      break;
   case CBQ24161Module::EBatteryState::DISCONNECTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "DIS");
      break;
   case CBQ24161Module::EBatteryState::UNDEFINED:
      fprintf(CFirmware::GetInstance().m_psHUART, "UNDEF");
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
      fprintf(CFirmware::GetInstance().m_psHUART, "RDY");
      break;
   case CBQ24250Module::EDeviceState::CHARGING:
      fprintf(CFirmware::GetInstance().m_psHUART, "CHG");
      break;
   case CBQ24250Module::EDeviceState::DONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "DONE");
      break;
   case CBQ24250Module::EDeviceState::FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "FAULT");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, "fault: ");
   switch(m_cActuatorPowerManager.GetFault()) {
   case CBQ24250Module::EFault::NONE:
      fprintf(CFirmware::GetInstance().m_psHUART, "NA");
      break;
   case CBQ24250Module::EFault::INPUT_OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "IN_OV");
      break;
   case CBQ24250Module::EFault::INPUT_UNDER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "IN_UV");
      break;
   case CBQ24250Module::EFault::SLEEP:
      fprintf(CFirmware::GetInstance().m_psHUART, "SLP");
      break;
   case CBQ24250Module::EFault::BATT_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "BTHM_SHDN");
      break;
   case CBQ24250Module::EFault::BATT_OVER_VOLTAGE:
      fprintf(CFirmware::GetInstance().m_psHUART, "B_OV");
      break;
   case CBQ24250Module::EFault::DEV_THERMAL_SHDN:
      fprintf(CFirmware::GetInstance().m_psHUART, "DTHM_SHDN");
      break;
   case CBQ24250Module::EFault::DEV_TIMER_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "DTMR_FAIL");
      break;
   case CBQ24250Module::EFault::BATT_DISCONNECTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "NO_BATT");
      break;
   case CBQ24250Module::EFault::ISET_SHORTED:
      fprintf(CFirmware::GetInstance().m_psHUART, "ISET_FAIL");
      break;
   case CBQ24250Module::EFault::INPUT_FAULT:
      fprintf(CFirmware::GetInstance().m_psHUART, "IN_FAIL");
      break;
   case CBQ24250Module::EFault::UNDEFINED:
      fprintf(CFirmware::GetInstance().m_psHUART, "UNDEF");
      break;
   }
   fprintf(CFirmware::GetInstance().m_psHUART, "\r\n");
   fprintf(CFirmware::GetInstance().m_psHUART, 
           "WD EN = %c\r\n", 
           m_cActuatorPowerManager.GetWatchdogEnabled()?'t':'f');
   fprintf(CFirmware::GetInstance().m_psHUART, 
           "WD FAULT = %c\r\n", 
           m_cActuatorPowerManager.GetWatchdogFault()?'t':'f');
}


