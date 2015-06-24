#ifndef BQ24161_CONTROLLER_H
#define BQ24161_CONTROLLER_H

#include <stdint.h>

class CBQ24161Controller {
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

   enum class EUSBInputLimit {
      L100,
      L150,
      L500,
      L800,
      L900,
      L1500
   };

   enum class EAdapterInputLimit {
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
   //void SetAdapterInputLimit(EAdapterInputLimit)

   void DumpRegister(uint8_t un_addr);

   void SetUSBInputLimit(EUSBInputLimit e_usb_input_limit);

   void SetChargingEnable(bool b_enable);

   void SetNoBattOperationEnable(bool b_enable);

   void SetBatteryRegulationVoltage(uint16_t un_batt_voltage_mv);

   void SetBatteryChargingCurrent(uint16_t un_batt_chrg_current_ma);

   void SetBatteryTerminationCurrent(uint16_t un_batt_term_current_ma);

   void ResetWatchdogTimer();

   EFault GetFault() {
      return eFault;
   }

   ESource GetSelectedSource() {
      return eSelectedSource;
   }
   
   EDeviceState GetDeviceState() {
      return eDeviceState;
   }

   EInputState GetUSBInputState() {
      return eUSBInputState;
   }

   EInputState GetAdapterInputState() {
      return eAdapterInputState;
   }
   
   EBatteryState GetBatteryState() {
      return eBatteryState;
   }

private:
   EFault eFault;
   ESource eSelectedSource;
   EDeviceState eDeviceState;
   EInputState eAdapterInputState, eUSBInputState;
   EBatteryState eBatteryState;
};

#endif
