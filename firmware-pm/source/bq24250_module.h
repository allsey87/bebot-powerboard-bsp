#ifndef BQ24250_MODULE_H
#define BQ24250_MODULE_H

#include <stdint.h>

class CBQ24250Module {
public:

   enum class EFault {
      NONE,
      INPUT_OVER_VOLTAGE,
      INPUT_UNDER_VOLTAGE,
      SLEEP,
      BATT_THERMAL_SHDN,
      BATT_OVER_VOLTAGE,
      DEV_THERMAL_SHDN,
      DEV_TIMER_FAULT,
      BATT_DISCONNECTED,
      ISET_SHORTED,
      INPUT_FAULT,
      UNDEFINED
   };

   enum class EDeviceState {
      READY,
      CHARGING,
      DONE,
      FAULT
   };

   enum class EInputCurrentLimit {
      L100,
      L150,
      L500,
      L900,
      L1500,
      L2000,
      LEXT,
      LPTM,
      LHIZ
   };


   void EnableCharging();
   void SetInputCurrentLimit(EInputCurrentLimit eInputCurrentLimit);
   
   void DumpRegister(uint8_t un_addr);

   void SetRegisterValue(uint8_t un_addr, uint8_t un_mask, uint8_t un_value);
   uint8_t GetRegisterValue(uint8_t un_addr, uint8_t un_mask);

   void ResetWatchdogTimer();

   bool GetWatchdogEnabled() {
      return bWatchdogEnabled;
   }

   bool GetWatchdogFault() {
      return bWatchdogFault;
   }

   void Synchronize();

   EFault GetFault() {
      return eFault;
   }
    
   EDeviceState GetDeviceState() {
      return eDeviceState;
   }

private:
   EFault eFault;
   EDeviceState eDeviceState;
   bool bWatchdogEnabled;
   bool bWatchdogFault;
};

#endif

