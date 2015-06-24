#ifndef USB_INTERFACE_SYSTEM_H
#define USB_INTERFACE_SYSTEM_H

#include <usb2532_module.h>
#include <mcp23008_module.h>

#include <stdint.h>

class CUSBInterfaceSystem {
public:
   CUSBInterfaceSystem();

   void Enable();

   void Disable();

private:
   CMCP23008Module cMCP23008Module;
   CUSB2532Module cUSB2532Module;

};

#endif
