#ifndef BQ24161_CONTROLLER_H
#define BQ24161_CONTROLLER_H

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

   enum class EBatteryState {
      NORMAL,
      OVER_VOLTAGE,
      DISCONNECTED,
      UNDEFINED
   };
   
   void Synchronize();

   //void SetPreferredSource(ESource); //USB / ADAPTER


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
