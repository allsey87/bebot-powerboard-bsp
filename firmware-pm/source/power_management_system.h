#ifndef POWER_MANAGEMENT_SYSTEM_H
#define POWER_MANAGEMENT_SYSTEM_H

#define SYS_BATT_REG_VOLTAGE 4200
#define SYS_BATT_CHG_CURRENT 740
#define SYS_BATT_TRM_CURRENT 50

#define ACT_BATT_REG_VOLTAGE
#define ACT_BATT_CHG_CURRENT
#define ACT_BATT_TRM_CURRENT

#include <stdint.h>
#include <bq24161_module.h>
#include <bq24250_module.h>
#include <adc_controller.h>
#include <interrupt.h>

class CPowerManagementSystem {
public:
   CPowerManagementSystem();

   void Init();

   void SetSystemPowerOn(bool b_set_power_on);

   void SetActuatorPowerOn(bool b_set_power_on);

   void SetSystemToActuatorPassthroughPowerOn(bool b_set_power_on);

   bool IsUSBConnected();

   void Update();

   void PrintStatus();

private:
   CADCController m_cADCController;

   CBQ24161Module m_cSystemPowerManager;
   CBQ24250Module m_cActuatorPowerManager;

   CPCA9633Module m_cBatteryStatusLEDs;
   CPCA9633Module m_cInputStatusLEDs;

   uint16_t m_unSystemBatteryVoltage;
   uint16_t m_unActuatorBatteryVoltage;
};

#endif
