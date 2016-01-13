#ifndef USB_INTERFACE_SYSTEM_H
#define USB_INTERFACE_SYSTEM_H

#include <usb2532_module.h>
#include <mcp23008_module.h>

#include <stdint.h>

class CUSBInterfaceSystem {
public:
   enum class EUSBChargerType {
      DCP, CDP, SDP, SE1L, SE1H, SE1S, WAIT, DISABLED
   };

public:
   static CUSBInterfaceSystem& GetInstance();

   bool IsHighSpeedMode();

   bool IsSuspended();

   bool IsEnabled();
   
   void Enable();
   
   void Disable();

   EUSBChargerType GetUSBChargerType();

private:
   CUSBInterfaceSystem();
   
   static CUSBInterfaceSystem m_cInstance;

   CMCP23008Module cMCP23008Module;
   CUSB2532Module cUSB2532Module;

};

#endif
