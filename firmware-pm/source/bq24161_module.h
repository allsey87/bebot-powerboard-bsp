#ifndef BQ24161_MODULE_H
#define BQ24161_MODULE_H

#include <stdint.h>

class CBQ24161Module {
public:

   enum class EFault {
      NONE,
      DEV_THERMAL_SHDN,
      BATT_THERMAL_SHDN,
      WATCHDOG_TMR_EXPR,
      SAFETY_TMR_EXPR,
      ADAPTER_FAULT,
      USB_FAULT,
      BATT_FAULT
   };

   enum class ESource {
      NONE,
      ADAPTER,
      USB
   };   

   enum class EDeviceState {
      STANDBY,
      READY,
      CHARGING,
      DONE,
      FAULT
   };

   enum class EInputState {
      NORMAL,
      OVER_VOLTAGE,
      WEAK_SOURCE,
      UNDER_VOLTAGE
   };

   enum class EInputLimit {
      L0,
      L100,
      L150,
      L500,
      L800,
      L900,
      L1500,
      L2500
   };

   enum class EBatteryState {
      NORMAL,
      OVER_VOLTAGE,
      DISCONNECTED,
      UNDEFINED
   };
   
   void Synchronize();

   //void SetPreferredSource(ESource); //USB / ADAPTER

   void DumpRegister(uint8_t un_addr);

   void SetInputLimit(ESource e_source, EInputLimit e_input_limit);

   EInputLimit GetInputLimit(ESource e_source);

   void SetChargingEnable(bool b_enable);

   void SetNoBattOperationEnable(bool b_enable);

   void SetBatteryRegulationVoltage(uint16_t un_batt_voltage_mv);

   void SetBatteryChargingCurrent(uint16_t un_batt_chrg_current_ma);

   void SetBatteryTerminationCurrent(uint16_t un_batt_term_current_ma);

   void ResetWatchdogTimer();

   EFault GetFault();

   ESource GetSelectedSource();

   ESource GetPreferredSource();
   
   EDeviceState GetDeviceState();

   EInputState GetInputState(ESource e_source);
   
   EBatteryState GetBatteryState();

private:
   EFault eFault;
   ESource eSelectedSource;
   ESource ePreferredSource;
   EDeviceState eDeviceState;
   EInputState eAdapterInputState, eUSBInputState;
   EBatteryState eBatteryState;
};

#endif
